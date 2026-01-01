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
	ImGui::SetCurrentContext(settingsContext);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(settingsContext);

	//TODO: fix this
	//ImGui::SetCurrentContext(visualizerContext);
	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext(visualizerContext);
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
	static Settings& settings = Visualizer::GetSettings();

	if (window == settingsWindow)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		if (ImGui::Begin("Settings", NULL, flags))
		{
			static const char* directions[] = { "Up", "Down", "Left", "Right" };
			static const std::vector<char*>& textures = Resources::GetTextureNames();
			static I32 direction = (I32)settings.scrollDirection;
			static I32 tomId = settings.tomTexture->id;
			static I32 cymbalId = settings.cymbalTexture->id;
			static I32 kickId = settings.kickTexture->id;

			ImGui::Text("Press F1 to toggle config mode to drag and resize visualizer window");

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Scroll Speed:");
			ImGui::SameLine();
			ImGui::SliderFloat("##ScrollSpeed", &settings.scrollSpeed, 0.25f, 5.0f, "%.2f");

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Note Width:");
			ImGui::SameLine();
			ImGui::SliderFloat("##NoteWidth", &settings.noteWidth, 0.01f, 0.125f, "%.3f");

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Note Height:");
			ImGui::SameLine();
			ImGui::SliderFloat("##NoteHeight", &settings.noteHeight, 0.01f, 0.125f, "%.3f");

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Scroll Direction:");
			ImGui::SameLine();
			if (ImGui::Combo("##ScrollDirection", &direction, directions, IM_COUNTOF(directions)))
			{
				Visualizer::SetScrollDirection((ScrollDirection)direction);
			}

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
	}
	else
	{
		if (ImGui::Begin("Settings", NULL, flags))
		{

		}
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}