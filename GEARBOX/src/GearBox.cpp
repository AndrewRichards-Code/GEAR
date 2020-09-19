#include "GearBox.h"

using namespace miru;
using namespace miru::crossplatform;

using namespace gear;
using namespace graphics;
using namespace scene;

using namespace mars;

GearBox::GearBox(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	ui.mainView->setAttribute(Qt::WA_NativeWindow);
	ui.mainView->setAttribute(Qt::WA_PaintOnScreen);
	ui.mainView->setAttribute(Qt::WA_NoSystemBackground);

	ui.actionEnglish->setData("en");
	ui.actionJapanese->setData("ja");
	QActionGroup* langGroup = new QActionGroup(ui.menuLanguage);
	langGroup->addAction(ui.actionEnglish);
	langGroup->addAction(ui.actionJapanese);
	connect(langGroup, SIGNAL(triggered(QAction*)), this, SLOT(LanguageChanged(QAction*)));

	timer = new QTimer(this);
	timer->setSingleShot(false);
	timer->start(1);
	connect(timer, SIGNAL(timeout()), this, SLOT(Render()));
	connect(ui.playButton, SIGNAL(clicked()), this, SLOT(PlayScene()));
	connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(StopScene()));
	connect(ui.reloadScriptButton, SIGNAL(clicked()), this, SLOT(ReloadScripts()));

	m_RenderSurfaceCI.debugName = this->windowTitle().toStdString();;
	m_RenderSurfaceCI.window = (HWND)ui.mainView->winId();
	m_RenderSurfaceCI.api = GraphicsAPI::API::VULKAN;
	m_RenderSurfaceCI.width = (uint32_t)ui.mainView->width();
	m_RenderSurfaceCI.height = (uint32_t)ui.mainView->height();
	m_RenderSurfaceCI.vSync = true;
	m_RenderSurfaceCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	m_RenderSurfaceCI.graphicsDebugger = debug::GraphicsDebugger::DebuggerType::NONE;
	m_RenderSurface = gear::CreateRef<RenderSurface>(&m_RenderSurfaceCI);
	UpdateRenderingLabels();

	MemoryBlockManager::CreateInfo mbmCI;
	mbmCI.pContext = m_RenderSurface->GetContext();
	mbmCI.defaultBlockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
	MemoryBlockManager::Initialise(&mbmCI);

	sceneCI = { "GEAR_TEST_Main_Scene", "res/scenes/current_scene.gsf.json" };
	activeScene = gear::CreateRef<Scene>(&sceneCI);

	renderer = gear::CreateRef<Renderer>(m_RenderSurface->GetContext());
	renderer->InitialiseRenderPipelines({ "res/pipelines/basic.grpf.json", "res/pipelines/cube.grpf.json" }, (float)m_RenderSurface->GetWidth(), (float)m_RenderSurface->GetHeight(), m_RenderSurface->GetRenderPass());

	Camera::CreateInfo cameraCI;
	cameraCI.debugName = "Main";
	cameraCI.device = m_RenderSurface->GetDevice();
	cameraCI.transform.translation = Vec3(0, 0, 0);
	cameraCI.transform.orientation = Quat(1, 0, 0, 0);
	cameraCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	cameraCI.projectionType = Camera::ProjectionType::PERSPECTIVE;
	cameraCI.perspectiveParams = { DegToRad(90.0), m_RenderSurface->GetRatio(), 0.01f, 3000.0f };
	cameraCI.flipX = false;
	cameraCI.flipY = false;
	cameraEnitity = activeScene->CreateEntity();
	cameraEnitity.AddComponent<CameraComponent>(std::move(gear::CreateRef<Camera>(&cameraCI)));
	cameraEnitity.AddComponent<NativeScriptComponent>("TestScript");

	Light::CreateInfo lightCI;
	lightCI.debugName = "Main";
	lightCI.device = m_RenderSurface->GetDevice();
	lightCI.type = Light::LightType::GEAR_LIGHT_POINT;
	lightCI.colour = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightCI.transform.translation = Vec3(0.0f, 1.0f, 0.0f);
	lightCI.transform.orientation = Quat(1, 0, 0, 0);
	lightCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	Entity light = activeScene->CreateEntity();
	light.AddComponent<LightComponent>(std::move(gear::CreateRef<Light>(&lightCI)));

	auto LoadTexture = [](void* device, std::string filepath, std::string debugName) -> gear::Ref<graphics::Texture>
	{
		Texture::CreateInfo texCI;
		texCI.debugName = debugName.c_str();
		texCI.device = device;
		texCI.filepaths = { filepath };
		texCI.mipLevels = 1;
		texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
		texCI.format = miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
		texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		texCI.usage = miru::crossplatform::Image::UsageBit(0);
		texCI.generateMipMaps = false;
		return std::move(gear::CreateRef<Texture>(&texCI));
	};

	std::future<gear::Ref<graphics::Texture>> rustIronAlbedo = std::async(std::launch::async, LoadTexture, m_RenderSurface->GetDevice(), std::string("res/img/rustediron2-Unreal-Engine/rustediron2_basecolor.png"), std::string("UE4 Rust Iron: Albedo"));
	std::future<gear::Ref<graphics::Texture>> rustIronMetallic = std::async(std::launch::async, LoadTexture, m_RenderSurface->GetDevice(), std::string("res/img/rustediron2-Unreal-Engine/rustediron2_metallic.png"), std::string("UE4 Rust Iron: Metallic"));
	std::future<gear::Ref<graphics::Texture>> rustIronNormal = std::async(std::launch::async, LoadTexture, m_RenderSurface->GetDevice(), std::string("res/img/rustediron2-Unreal-Engine/rustediron2_normal.png"), std::string("UE4 Rust Iron: Normal"));
	std::future<gear::Ref<graphics::Texture>> rustIronRoughness = std::async(std::launch::async, LoadTexture, m_RenderSurface->GetDevice(), std::string("res/img/rustediron2-Unreal-Engine/rustediron2_roughness.png"), std::string("UE4 Rust Iron: Roughness"));

	std::future<gear::Ref<graphics::Texture>> slateAlbedo = std::async(std::launch::async, LoadTexture, m_RenderSurface->GetDevice(), std::string("res/img/slate2-tiled-ue4/slate2-tiled-albedo2.png"), std::string("UE4 Slate: Albedo"));
	std::future<gear::Ref<graphics::Texture>> slateMetallic = std::async(std::launch::async, LoadTexture, m_RenderSurface->GetDevice(), std::string("res/img/slate2-tiled-ue4/slate2-tiled-metalness.png"), std::string("UE4 Slate: Metallic"));
	std::future<gear::Ref<graphics::Texture>> slateNormal = std::async(std::launch::async, LoadTexture, m_RenderSurface->GetDevice(), std::string("res/img/slate2-tiled-ue4/slate2-tiled-normal3-UE4.png"), std::string("UE4 Slate: Normal"));
	std::future<gear::Ref<graphics::Texture>> slateRoughness = std::async(std::launch::async, LoadTexture, m_RenderSurface->GetDevice(), std::string("res/img/slate2-tiled-ue4/slate2-tiled-rough.png"), std::string("UE4 Slate: Roughness"));
	std::future<gear::Ref<graphics::Texture>> slateAO = std::async(std::launch::async, LoadTexture, m_RenderSurface->GetDevice(), std::string("res/img/slate2-tiled-ue4/slate2-tiled-ao.png"), std::string("UE4 Slate: AO"));

	Material::CreateInfo matCI;
	matCI.debugName = "UE4 Rust Iron";
	matCI.device = m_RenderSurface->GetDevice();
	matCI.pbrTextures = {
		{ Material::TextureType::NORMAL, rustIronNormal.get() },
		{ Material::TextureType::ALBEDO, rustIronAlbedo.get() },
		{ Material::TextureType::METALLIC, rustIronMetallic.get() },
		{ Material::TextureType::ROUGHNESS, rustIronRoughness.get() },
	};
	gear::Ref<Material> rustIronMaterial = gear::CreateRef<Material>(&matCI);

	matCI.debugName = "UE4 Slate";
	matCI.device = m_RenderSurface->GetDevice();
	matCI.pbrTextures = {
		{ Material::TextureType::NORMAL, slateNormal.get() },
		{ Material::TextureType::ALBEDO, slateAlbedo.get() },
		{ Material::TextureType::METALLIC, slateMetallic.get() },
		{ Material::TextureType::ROUGHNESS, slateRoughness.get() },
		{ Material::TextureType::AMBIENT_OCCLUSION, slateAO.get() },
	};
	gear::Ref<Material> slateMaterial = gear::CreateRef<Material>(&matCI);

	Mesh::CreateInfo meshCI;
	meshCI.device = m_RenderSurface->GetDevice();
	meshCI.debugName = "quad.fbx";
	meshCI.filepath = "res/obj/quad.fbx";
	gear::Ref<Mesh> quadMesh = gear::CreateRef<Mesh>(&meshCI);
	quadMesh->SetOverrideMaterial(0, slateMaterial);

	meshCI.device = m_RenderSurface->GetDevice();
	meshCI.debugName = "sphere.fbx";
	meshCI.filepath = "res/obj/sphere.fbx";
	gear::Ref<Mesh> sphereMesh = gear::CreateRef<Mesh>(&meshCI);
	sphereMesh->SetOverrideMaterial(0, rustIronMaterial);

	Model::CreateInfo modelCI;
	modelCI.debugName = "Quad";
	modelCI.device = m_RenderSurface->GetDevice();
	modelCI.pMesh = quadMesh;
	modelCI.materialTextureScaling = Vec2(100.0f, 100.0f);
	modelCI.transform.translation = Vec3(0, 0, 0);
	modelCI.transform.orientation = Quat(sqrt(2) / 2, -sqrt(2) / 2, 0, 0);
	modelCI.transform.scale = Vec3(100.0f, 100.0f, 100.0f);
	modelCI.renderPipelineName = "basic";
	Entity quad = activeScene->CreateEntity();
	quad.AddComponent<ModelComponent>(std::move(gear::CreateRef<Model>(&modelCI)));

	modelCI.debugName = "Sphere";
	modelCI.device = m_RenderSurface->GetDevice();
	modelCI.pMesh = sphereMesh;
	modelCI.materialTextureScaling = Vec2(1.0f, 1.0f);
	modelCI.transform.translation = Vec3(0, 1.0f, -1.5);
	modelCI.transform.orientation = Quat(1, 0, 0, 0);
	modelCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	modelCI.renderPipelineName = "basic";
	Entity sphere = activeScene->CreateEntity();
	sphere.AddComponent<ModelComponent>(std::move(gear::CreateRef<Model>(&modelCI)));
};

GearBox::~GearBox()
{
	delete timer;
}

void GearBox::changeEvent(QEvent* event)
{
	if (event) 
	{
		switch (event->type()) 
		{
		case QEvent::LanguageChange:
			ui.retranslateUi(this);
			break;
		default:
			break;
		}
	}
	QMainWindow::changeEvent(event);
}

void GearBox::Render()
{
	//Update from Window
	if (ui.mainView->width() != m_RenderSurface->GetWidth() || ui.mainView->height() != m_RenderSurface->GetHeight())
	{
		m_RenderSurface->Resize(ui.mainView->width(), ui.mainView->height());
		if (m_RenderSurface->Resized())
		{
			renderer->ResizeRenderPipelineViewports((uint32_t)m_RenderSurface->GetWidth(), (uint32_t)m_RenderSurface->GetHeight());
			m_RenderSurface->Resized() = false;
		}
	}

	//Camera Update
	auto& camera = cameraEnitity.GetComponent<CameraComponent>().camera;
	camera->m_CI.transform.translation.y = 1.0f;
	camera->Update();

	//Update Scene
	activeScene->OnUpdate(renderer, gearTimer);

	renderer->SubmitFramebuffer(m_RenderSurface->GetFramebuffers());
	renderer->Upload(true, false, false);
	renderer->Flush();

	renderer->Present(m_RenderSurface->GetSwapchain(), windowResize);
}

void GearBox::LanguageChanged(QAction* action)
{
	QString locale = action->data().toString();
	
	qApp->removeTranslator(&translator);
	
	bool loaded = translator.load(QString("gearbox_") + locale + QString(".qm"));
	if (loaded)
		qApp->installTranslator(&translator);

}

void GearBox::PlayScene()
{
	activeScene->Play();
}

void GearBox::StopScene()
{
	activeScene->Stop();
}

void GearBox::UpdateRenderingLabels()
{
	QString apiName = QString(m_RenderSurface->GetGraphicsAPIVersion().c_str());
	ui.renderingAPILabel->setText(apiName);
	QString gpuName = QString(m_RenderSurface->GetDeviceName().c_str());
	ui.gpuDeviceLabel->setText(gpuName);
}

void GearBox::ReloadScripts()
{
	activeScene->UnloadNativeScriptLibrary();
	activeScene->LoadNativeScriptLibrary();
}
