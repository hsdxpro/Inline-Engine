#include "Node_DrawSky.hpp"

#include <array>

// OBSOLETE
// please update this class to match current pipeline

#if 0
namespace inl::gxeng::nodes {


DrawSky::DrawSky(gxapi::IGraphicsApi * graphicsApi):
	m_binder(graphicsApi, {})
{
	BindParameterDesc cbBindParamDesc;
	m_cbBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
	cbBindParamDesc.parameter = m_cbBindParam;
	cbBindParamDesc.constantSize = sizeof(float) * 4 * 3 + sizeof(float)*16 + sizeof(float)*4;
	cbBindParamDesc.relativeAccessFrequency = 0;
	cbBindParamDesc.relativeChangeFrequency = 0;
	cbBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

	BindParameterDesc sampBindParamDesc;
	sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
	sampBindParamDesc.constantSize = 0;
	sampBindParamDesc.relativeAccessFrequency = 0;
	sampBindParamDesc.relativeChangeFrequency = 0;
	sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	gxapi::StaticSamplerDesc samplerDesc;
	samplerDesc.shaderRegister = 0;
	samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.mipLevelBias = 0.f;
	samplerDesc.registerSpace = 0;
	samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	m_binder = Binder{ graphicsApi,{ cbBindParamDesc, sampBindParamDesc },{ samplerDesc } };
}


void DrawSky::InitGraphics(const GraphicsContext & context) {
	m_graphicsContext = context;

	std::vector<float> vertices = {
		-1, -1, 0,
		+1, -1, 0,
		+1, +1, 0,
		-1, +1, 0
	};
	std::vector<uint16_t> indices = {
		0, 1, 2,
		0, 2, 3
	};
	m_fsq = m_graphicsContext.CreateVertexBuffer(vertices.data(), sizeof(float)*vertices.size());
	m_fsqIndices = m_graphicsContext.CreateIndexBuffer(indices.data(), sizeof(uint16_t)*indices.size(), indices.size());

	ShaderParts shaderParts;
	shaderParts.vs = true;
	shaderParts.ps = true;

	auto shader = m_graphicsContext.CreateShader("DrawSky", shaderParts, "");

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0)
	};

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = m_binder.GetRootSignature();
	psoDesc.vs = shader.vs;
	psoDesc.ps = shader.ps;
	psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

	psoDesc.depthStencilState.enableDepthTest = false;
	psoDesc.depthStencilState.enableDepthStencilWrite = false;
	psoDesc.depthStencilState.enableStencilTest = true;
	psoDesc.depthStencilState.stencilReadMask = ~uint8_t(0);
	psoDesc.depthStencilState.stencilWriteMask = 0;
	psoDesc.depthStencilState.ccwFace.stencilFunc = gxapi::eComparisonFunction::EQUAL;
	psoDesc.depthStencilState.ccwFace.stencilOpOnStencilFail = gxapi::eStencilOp::KEEP;
	psoDesc.depthStencilState.ccwFace.stencilOpOnDepthFail = gxapi::eStencilOp::KEEP;
	psoDesc.depthStencilState.ccwFace.stencilOpOnPass = gxapi::eStencilOp::KEEP;
	psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;
	psoDesc.depthStencilFormat = gxapi::eFormat::D32_FLOAT_S8X24_UINT;
	
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R16G16B16A16_FLOAT;

	m_PSO.reset(m_graphicsContext.CreatePSO(psoDesc));
}


void DrawSky::Render(
	RenderTargetView2D& rtv,
	DepthStencilView2D& dsv,
	const Camera* camera,
	const DirectionalLight* sun,
	GraphicsCommandList& commandList
) {
	auto* pRTV = &rtv;
	commandList.SetResourceState(rtv.GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(dsv.GetResource(), 0, gxapi::eResourceState::DEPTH_READ);
	commandList.SetRenderTargets(1, &pRTV, &dsv);

	gxapi::Rectangle rect{ 0, (int)pRTV->GetResource().GetHeight(), 0, (int)pRTV->GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder);
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
	commandList.SetStencilRef(0); // only allow sky to be rendered to background pixels

	gxeng::VertexBuffer* pVertexBuffer = &m_fsq;
	unsigned vbSize = m_fsq.GetSize();
	unsigned vbStride = 3 * sizeof(float);

	struct Sun {
		mathfu::VectorPacked<float, 4> invViewProj[4];
		mathfu::VectorPacked<float, 4> dirView;
		mathfu::VectorPacked<float, 4> dirWorld;
		mathfu::VectorPacked<float, 4> color;
		mathfu::VectorPacked<float, 4> viewPos;
	};

	Sun sunCB;

	mathfu::Matrix4x4f viewInvTr = camera->GetViewMatrixRH().Inverse().Transpose();
	mathfu::Vector4f sunViewDir = viewInvTr * mathfu::Vector4f(sun->GetDirection(), 0.0f);
	mathfu::Vector4f sunColor = mathfu::Vector4f(sun->GetColor(), 1.0f);
	mathfu::Matrix4x4f invViewProj = (camera->GetPerspectiveMatrixRH() * camera->GetViewMatrixRH()).Inverse();

	sunCB.dirView = sunViewDir;
	sunCB.dirWorld = mathfu::Vector4f(sun->GetDirection(), 0.0);
	sunCB.color = sunColor;
	invViewProj.Pack(sunCB.invViewProj);
	sunCB.viewPos = mathfu::Vector4f(camera->GetPosition(), 1);

	commandList.BindGraphics(m_cbBindParam, &sunCB, sizeof(sunCB), 0);
	commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
	commandList.SetIndexBuffer(&m_fsqIndices, false);
	commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
}


} // namespace inl::gxeng::nodes
#endif
