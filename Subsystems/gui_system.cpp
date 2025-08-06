#include "gui_system.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <tchar.h>

GuiSystem::GuiSystem() {}
GuiSystem::GuiSystem(const GuiSystem& other) {}
GuiSystem::~GuiSystem() {}

bool GuiSystem::Initialize(HWND hwnd, RenderSystem* renderSystem)
{
	m_renderSystem = renderSystem;
	
	// Create application window
	ImGui_ImplWin32_EnableDpiAwareness();

	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_io = &ImGui::GetIO(); (void)m_io;
	m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Setup ImGui style
	ImGui::StyleColorsDark();

	// Setup backends
	ImGui_ImplWin32_Init(hwnd);

	return true; // TEMP
}

void GuiSystem::Shutdown()
{
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool GuiSystem::Frame(RenderParameters& rParams)
{
	// Start the ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	static char filename[256] = "";

	// Show window
	{
		ImGui::Begin("Editor Controls");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_io->Framerate, m_io->Framerate);

		if (ImGui::BeginTabBar("Tabs"))
		{
			if (ImGui::BeginTabItem("Scene Controls"))
			{
				ImGui::DragFloat("Vertical FOV", &rParams.verticalFOV, 0.2f, 10, 90);
				ImGui::DragFloat("Near Z", &rParams.nearZ, 0.002f, 0.001, 10);
				ImGui::DragFloat("Far Z", &rParams.farZ, 0.2f, 10, 1000);

				ImGui::DragFloat3("Light Direction", rParams.lightDirection, 0.004f);
				ImGui::ColorEdit3("Clear Color", rParams.clearColor);
				ImGui::DragFloat("Ambient Strength", &rParams.ambientStrength, 0.002f, 0, 1);

				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem("Pass Controls"))
			{
				const auto& effect = m_renderSystem->m_effect;
				std::vector<const char*> keys;
				for (const auto& pair : effect->GetTargets())
					keys.push_back(pair.first.data());
				
				auto params = m_renderSystem->m_outPass->GetParameters();
				auto& pTarget = std::get<std::reference_wrapper<std::string>>(params[0].m_field).get();
				int selected;
				for (selected = 0; selected < keys.size(); ++selected)
					if (keys[selected] == pTarget)
						break;

				if (selected == keys.size())
					selected = 0;
				ImGui::Combo("Output Target", &selected, keys.data(), keys.size());

				ImGui::Separator();

				pTarget = keys[selected];
				for (const auto& param : m_renderSystem->m_geometryPass->GetParameters())
				{
					switch (param.m_type)
					{
					case RenderPass::WidgetType::CHECKBOX:
					{
						auto& pBool = std::get<std::reference_wrapper<int>>(param.m_field).get();
						bool b = (pBool != 0);
						ImGui::Checkbox(param.m_name.data(), &b);

						pBool = b ? 1 : 0;
						break;
					}
					case RenderPass::WidgetType::FLOAT:
					{
						auto& pFloat = std::get<std::reference_wrapper<float>>(param.m_field).get();
						ImGui::DragFloat(param.m_name.data(), &pFloat, 0.002f, 0, 1);
						break;
					}
					case RenderPass::WidgetType::COLOR:
					{
						auto& pColor = std::get<std::reference_wrapper<XMFLOAT3>>(param.m_field).get();
						float color[3] = {pColor.x, pColor.y, pColor.z};
						ImGui::ColorEdit3(param.m_name.data(), color);
						pColor = { color[0], color[1], color[2] };
						break;
					}
					}
				}
				ImGui::Separator();
				

				for (const auto& p : effect->GetPasses())
				{
					for (const auto& param : p->GetParameters())
					{
						switch (param.m_type)
						{
						case RenderPass::WidgetType::RENDER_TARGET:
						{
							auto& pTarget = std::get<std::reference_wrapper<std::string>>(param.m_field).get();
							int selected;
							for (selected = 0; selected < keys.size(); ++selected)
								if (keys[selected] == pTarget)
									break;

							if (selected == keys.size())
								selected = 0;

							ImGui::Combo(param.m_name.data(), &selected, keys.data(), keys.size());
							pTarget = keys[selected];

							break;
						}
						case RenderPass::WidgetType::CHECKBOX:
						{
							auto& pBool = std::get<std::reference_wrapper<int>>(param.m_field).get();
							bool b = (pBool != 0);
							ImGui::Checkbox(param.m_name.data(), &b);

							pBool = b ? 1 : 0;
							break;
						}
						case RenderPass::WidgetType::INT:
						{
							auto& pInt = std::get<std::reference_wrapper<int>>(param.m_field).get();
							ImGui::DragInt(param.m_name.data(), &pInt, 1, 0, 8);
							break;
						}
						case RenderPass::WidgetType::FLOAT:
						{
							auto& pFloat = std::get<std::reference_wrapper<float>>(param.m_field).get();
							ImGui::DragFloat(param.m_name.data(), &pFloat, 0.002f, 0, 1);
							break;
						}
						case RenderPass::WidgetType::FLOAT3:
						{
							auto& pFloat3 = std::get<std::reference_wrapper<XMFLOAT3>>(param.m_field).get();
							float float3[3] = { pFloat3.x, pFloat3.y, pFloat3.z };
							ImGui::DragFloat3(param.m_name.data(), float3, 0.004f);
							pFloat3 = { float3[0], float3[1], float3[2] };
							break;
						}
						case RenderPass::WidgetType::COLOR:
						{
							auto& pColor = std::get<std::reference_wrapper<XMFLOAT3>>(param.m_field).get();
							float color[3] = { pColor.x, pColor.y, pColor.z };
							ImGui::ColorEdit3(param.m_name.data(), color);
							pColor = { color[0], color[1], color[2] };
							break;
						}
						}
					}

					ImGui::Separator();
				}

				// Temp creators
				if (ImGui::Button("Create Lighting Pass"))
					effect->AddPass(std::make_unique<LightingPass>());

				if (ImGui::Button("Create Canny Pass"))
					effect->AddPass(std::make_unique<CannyPass>());

				if (ImGui::Button("Create Blend Pass"))
					effect->AddPass(std::make_unique<BlendPass>());


				ImGui::EndTabItem();
			}
			
			ImGui::EndTabBar();
		}



		ImGui::End();
	}

	ImGui::Render();

	return true;
}