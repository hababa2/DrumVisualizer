#include "UI.hpp"

#include "GraphicsInclude.hpp"
#include "Visualizer.hpp"

#include <iostream>

Window* UI::settingsWindow;
Window* UI::visualizerWindow;

ImGuiContext* UI::settingsContext;
ImGuiContext* UI::visualizerContext;

ImGuiWindowFlags UI::flags;
Settings* UI::settings;
std::array<Stats, 8>* UI::stats;
std::array<NoteInfo, 8>* UI::noteInfos;
const std::vector<char*>* UI::ports;

const char* UI::directions[] = { "Up", "Down", "Left", "Right" };
const std::vector<char*>* UI::textures;
I32 UI::direction;
I32 UI::tomId;
I32 UI::cymbalId;
I32 UI::kickId;
I32 UI::portId;

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

	flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
	settings = &Visualizer::GetSettings();
	stats = &Visualizer::GetStats();
	noteInfos = &Visualizer::GetNoteInfos();
	ports = &Visualizer::GetPorts();

	textures = &Resources::GetTextureNames();
	direction = (I32)settings->scrollDirection;
	tomId = settings->tomTexture->id;
	cymbalId = settings->cymbalTexture->id;
	kickId = settings->kickTexture->id;

	I32 i = 0;
	for (const std::string& port : *ports)
	{
		if (port == settings->portName)
		{
			portId = i;
		}

		++i;
	}

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
	if (window == settingsWindow)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		if (ImGui::Begin("Settings", NULL, flags))
		{
			ImGui::Text("Press F1 to toggle config mode to drag and resize visualizer window");

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Show Stats:");
			ImGui::SameLine();
			if (ImGui::Checkbox("##ShowStats", &settings->showStats))
			{
				Visualizer::SetScrollDirection((ScrollDirection)direction);
			}

			ImGui::SameLine();
			if (ImGui::Button("Reset Stats"))
			{
				for (Stats& s : *stats)
				{
					s.hitCount = 0;
					s.ghostCount = 0;
				}
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Show Dynamics:");
			ImGui::SameLine();
			ImGui::Checkbox("##ShowDynamics", &settings->showDynamics);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Scroll Speed:");
			ImGui::SameLine();
			ImGui::SliderFloat("##ScrollSpeed", &settings->scrollSpeed, 0.25f, 5.0f, "%.2f");

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Note Width:");
			ImGui::SameLine();
			ImGui::SliderFloat("##NoteWidth", &settings->noteWidth, 0.01f, 0.125f, "%.3f");

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Note Height:");
			ImGui::SameLine();
			ImGui::SliderFloat("##NoteHeight", &settings->noteHeight, 0.01f, 0.125f, "%.3f");

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
			if (ImGui::Combo("##TomTexture", &tomId, textures->data(), (I32)textures->size()))
			{
				settings->tomTextureName = textures->at(tomId);
				settings->tomTexture = Resources::GetTexture(settings->tomTextureName);
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Cymbal Texture:");
			ImGui::SameLine();
			if (ImGui::Combo("##CymbalTexture", &cymbalId, textures->data(), (I32)textures->size()))
			{
				settings->cymbalTextureName = textures->at(cymbalId);
				settings->cymbalTexture = Resources::GetTexture(settings->cymbalTextureName);
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Kick Texture:");
			ImGui::SameLine();
			if (ImGui::Combo("##KickTexture", &kickId, textures->data(), (I32)textures->size()))
			{
				settings->kickTextureName = textures->at(kickId);
				settings->kickTexture = Resources::GetTexture(settings->kickTextureName);
			}

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Midi Ports:");
			ImGui::SameLine();
			if (ImGui::Combo("##MidiPorts", &portId, ports->data(), (I32)ports->size()))
			{
				settings->portName = ports->at(portId);
				Visualizer::LoadPort(settings->portName);
			}

			const F32 size = 70.0f;
			const F32 spacing = 8.0f;

			ImGui::Text("Rearrange Layout:");

			for (U32 i = 0; i < noteInfos->size(); ++i)
			{
				ImGui::PushID(i);
				if(i > 0) { ImGui::SameLine(0, spacing); }

				ImGui::Button(noteInfos->at(i).name.c_str(), ImVec2(size, size));

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover))
				{
					ImGui::SetDragDropPayload("DND_SQUARE", &i, sizeof(I32));

					ImGui::Text("Moving %s", noteInfos->at(i).name.c_str());
					ImGui::Button(noteInfos->at(i).name.c_str(), ImVec2(size, size));

					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SQUARE"))
					{
						I32 source = *(const I32*)payload->Data;
						I32 target = i;

						if (source < target)
						{
							std::rotate(noteInfos->begin() + source,
								noteInfos->begin() + source + 1,
								noteInfos->begin() + target + 1);
						}
						else if (source > target)
						{
							std::rotate(noteInfos->begin() + target,
								noteInfos->begin() + source,
								noteInfos->begin() + source + 1);
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::PopID();
			}
		}

		ImGui::End();
	}
	else if (settings->showStats)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 size = viewport->Size;
		ImVec2 pos = viewport->Pos;
		bool horizontal = true;

		switch (settings->scrollDirection)
		{
		case ScrollDirection::Up: {
			pos.y = size.y - 50.0f;
			size.y = 50.0f;
			horizontal = true;
		} break;
		case ScrollDirection::Down: {
			size.y = 50.0f;
			horizontal = true;
		} break;
		case ScrollDirection::Left: {
			pos.x = size.x - 50.0f;
			size.x = 50.0f;
			horizontal = false;
		} break;
		case ScrollDirection::Right: {
			size.x = 50.0f;
			horizontal = false;
		} break;
		}

		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);

		if (ImGui::Begin("Settings", NULL, flags))
		{
			if (horizontal)
			{
				if (ImGui::BeginTable("##StatsTable", 8))
				{
					F32 rowHeight = ImGui::GetContentRegionAvail().y;
					F32 lineSpacing = ImGui::GetStyle().ItemSpacing.y;
					F32 textLineHeight = ImGui::GetTextLineHeight();
					F32 blockHeight = settings->showDynamics ? (textLineHeight * 2) + lineSpacing : textLineHeight;

					for (NoteInfo& note : *noteInfos)
					{
						Stats& s = stats->at(note.index);
						SetupColumn(s.hitCount, s.ghostCount, rowHeight, blockHeight, settings->showDynamics);
					}

					ImGui::EndTable();
				}
			}
			else
			{
				if (ImGui::BeginTable("##StatsTable", 1))
				{
					F32 availableHeight = ImGui::GetContentRegionAvail().y;
					F32 rowHeight = availableHeight / 8.0f;
					F32 lineSpacing = ImGui::GetStyle().ItemSpacing.y;
					F32 textLineHeight = ImGui::GetTextLineHeight();
					F32 blockHeight = settings->showDynamics ? (textLineHeight * 2) + lineSpacing : textLineHeight;

					for (I32 i = noteInfos->size() - 1; i >= 0; --i)
					{
						Stats& s = stats->at(noteInfos->at(i).index);
						SetupRow(s.hitCount, s.ghostCount, rowHeight, blockHeight, settings->showDynamics);
					}

					ImGui::EndTable();
				}
			}
		}

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::SetupColumn(U32 value1, U32 value2, F32 rowHeight, F32 blockHeight, bool showDynamics)
{
	ImGui::TableNextColumn();

	std::string text1 = std::to_string(value1);
	std::string text2 = std::to_string(value2);

	F32 startPosY = ImGui::GetCursorPosY() + (rowHeight - blockHeight) * 0.5f;

	ImGui::SetCursorPosY(startPosY);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(text1.c_str()).x) * 0.5f);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), text1.c_str());

	if (showDynamics)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(text2.c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), text2.c_str());
	}
}

void UI::SetupRow(U32 value1, U32 value2, F32 rowHeight, F32 blockHeight, bool showDynamics)
{
	ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
	ImGui::TableNextColumn();

	std::string text1 = std::to_string(value1);
	std::string text2 = std::to_string(value2);

	F32 startPosY = ImGui::GetCursorPosY() + (rowHeight - blockHeight) * 0.5f;

	ImGui::SetCursorPosY(startPosY);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(text1.c_str()).x) * 0.5f);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), text1.c_str());

	if (showDynamics)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(text2.c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), text2.c_str());
	}
}