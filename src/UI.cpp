#include "UI.hpp"

#include "GraphicsInclude.hpp"
#include "Visualizer.hpp"

#include <iostream>

Window* UI::settingsWindow;
Window* UI::visualizerWindow;

ImGuiContext* UI::settingsContext;
ImGuiContext* UI::visualizerContext;

bool UI::Initialize(Window* settingsWindow_, Window* visualizerWindow_)
{
#ifdef DV_DEBUG
	std::cout << "Initializing UI..." << std::endl;
#endif

	settingsWindow = settingsWindow_;
	visualizerWindow = visualizerWindow_;

	IMGUI_CHECKVERSION();

	settingsContext = ImGui::CreateContext();
	visualizerContext = ImGui::CreateContext();

	ImGui::SetCurrentContext(settingsContext);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(*settingsWindow, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGui::SetCurrentContext(visualizerContext);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(*visualizerWindow, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	return true;
}

void UI::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(settingsContext);
	ImGui::DestroyContext(visualizerContext);
}

void UI::Update(Window* window)
{
	if (window == settingsWindow) { ImGui::SetCurrentContext(settingsContext); }
	else { ImGui::SetCurrentContext(visualizerContext); }

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void UI::Render(Window* window)
{
	static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

	if (window == settingsWindow)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		if (ImGui::Begin("Settings", NULL, flags))
		{
			Settings& settings = Visualizer::GetSettings();
			const std::vector<char*>& textures = Resources::GetTextureNames();
			static int tomId = settings.tomTexture->id;
			static int cymbalId = settings.cymbalTexture->id;
			static int kickId = settings.kickTexture->id;

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Tom Texture:");
			ImGui::SameLine();
			if (ImGui::Combo("##TomTexture", &tomId, textures.data(), (I32)textures.size()))
			{
				settings.tomTextureName = textures[tomId];
				settings.tomTexture = Resources::GetTexture(settings.tomTextureName);
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Cymbal Texture:");
			ImGui::SameLine();
			if (ImGui::Combo("##CymbalTexture", &cymbalId, textures.data(), (I32)textures.size()))
			{
				settings.cymbalTextureName = textures[cymbalId];
				settings.cymbalTexture = Resources::GetTexture(settings.cymbalTextureName);
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Kick Texture:");
			ImGui::SameLine();
			if (ImGui::Combo("##KickTexture", &kickId, textures.data(), (I32)textures.size()))
			{
				settings.kickTextureName = textures[kickId];
				settings.kickTexture = Resources::GetTexture(settings.kickTextureName);
			}
		}

		ImGui::End();
	}
	else
	{

	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}