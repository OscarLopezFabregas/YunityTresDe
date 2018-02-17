#include "Application.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRenderer.h"

#include "imgui-1.53\imgui.h"
#include "imgui-1.53\imgui_impl_sdl_gl3.h"
#include "ModuleImGui.h"
#include "Mathgeolib\include\MathGeoLib.h"

#include "Brofiler/include/Brofiler.h"

#include "GameObject.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentCamera.h"
#include "ModuleCamera.h"

#include <map>

#define BOX_SIZE 20.0f

ModuleScene::ModuleScene()
{
}

ModuleScene::~ModuleScene()
{

}

bool ModuleScene::Init()
{

	root = new GameObject();
	GameObject *object1 = new GameObject();
	ComponentMesh *cm1 = new ComponentMesh(SPHERE);
	ComponentTransform *ct1 = new ComponentTransform(float3(0.0f,0.0f,0.0f), float3(1.0f,1.0f,1.0f), Quat::identity);
	object1->AddComponent(cm1);
	object1->AddComponent(ct1);
	root->AddChild(object1);
	sceneObjects_.push_back(object1);
	float offset = -2.0f;
	float xoff[16] = {20,20,0, -20,  0,-20,-20,20, 0 ,10,-10,0};
	float zoff[16] = {20,0, 20,-20,-20,0,20,-20, 10, 0, 0,-10};
  	for (int i = 0; i < 12; ++i) 
	{
		GameObject *object = new GameObject();
		ComponentMesh *cm = new ComponentMesh(CUBE);
		ComponentTransform *ct = new ComponentTransform(float3(0.0f+xoff[i], 0.0f, 0.0f+zoff[i]), float3(1.0f, 1.0f, 1.0f), Quat::identity);
		ComponentMaterial *material = new ComponentMaterial(object);
		object->AddComponent(cm);
		object->AddComponent(ct);
		object->AddComponent(material);
		root->AddChild(object);
		sceneObjects_.push_back(object);
		offset += offset;
		object->SetId(i+1);
	}
	actualCamera = App->cam->dummyCamera;

	return true;
}

bool ModuleScene::Start()
{
	limits = AABB();
	limits.maxPoint = float3(BOX_SIZE, BOX_SIZE, BOX_SIZE);
	limits.minPoint = float3(-BOX_SIZE, -BOX_SIZE, -BOX_SIZE);
	quadtree = new CustomQuadTree();
	quadtree->Create(limits);
	for (int i = 0; i < sceneObjects_.size(); ++i) quadtree->Insert(sceneObjects_[i]);
	quadtree->Intersect(objectToDraw_, *(actualCamera->GetFrustum()));
	return true;
}

bool ModuleScene::CleanUp()
{
	return true;
}

update_status ModuleScene::PreUpdate(float dt)
{
	ShowImguiStatus();
	ImGuiMainMenu();
	return UPDATE_CONTINUE;
}

update_status ModuleScene::Update(float dt)
{
	BROFILER_CATEGORY("UpdateModuleScene", Profiler::Color::Orchid);

	if (accelerateFrustumCulling) {
		if (recreateQuadTree) {
			quadtree->Clear();
			limits.maxPoint = float3(BOX_SIZE, BOX_SIZE, BOX_SIZE);
			limits.minPoint = float3(-BOX_SIZE, -BOX_SIZE, -BOX_SIZE);
			quadtree->Create(limits);
			for (int i = 0; i < sceneObjects_.size(); ++i) quadtree->Insert(sceneObjects_[i]);
		}
		quadtree->Intersect(objectToDraw_, *(actualCamera->GetFrustum()));
		for (int i = 0; i < objectToDraw_.size(); i++)
		{
			objectToDraw_[i]->DrawObjectAndChilds();
		}
		quadtree->DrawBox();
		objectToDraw_.clear();
	}
	else {
		for (int i = 0; i < sceneObjects_.size(); i++)
		{
			ComponentMesh* cm = (ComponentMesh*)sceneObjects_[i]->GetComponent(MESH);
			ComponentTransform* ct = (ComponentTransform*)sceneObjects_[i]->GetComponent(TRANSFORMATION);
			if (cm != nullptr && ct != nullptr) {
				AABB newBox = *(cm->GetBoundingBox());
				newBox.TransformAsAABB(ct->GetGlobalTransform());
				if (actualCamera->GetFrustum()->Intersects(newBox)) sceneObjects_[i]->DrawObjectAndChilds();
			}
		}
}

	//IMGUI
Hierarchy();
	

	return UPDATE_CONTINUE;
}

update_status ModuleScene::PostUpdate(float dt)
{
	if (imguiFlag == SDL_SCANCODE_ESCAPE)
	{
		return UPDATE_STOP;
	}
	return UPDATE_CONTINUE;
}

void ModuleScene::Hierarchy()
{
	ImGui::SetNextWindowPos(ImVec2(0, 20));
	ImGui::SetNextWindowSize(ImVec2(300, 680));
	ImGui::Begin("Hierarchy", 0, App->imgui->GetImGuiWindowFlags());
	static bool selected = false;
	for (int i = 0; i < sceneObjects_.size(); i++)
	{
	//	std::vector<static bool> selected;
		std::string  c = "Game Object " + std::to_string(i + 1);
		//Update vector
		if (sceneObjects_[i] == nullptr)
		{
			sceneObjects_.erase(sceneObjects_.begin()+i);
		}
		//---
		if (ImGui::Selectable((c.c_str()), &selected))
		{
			for (int j = 0; j < sceneObjects_.size(); j++)
			{
				sceneObjects_[j]->isSelected = false;
			}
			sceneObjects_[i]->isSelected = true;
		}
		//temporal, NEED FIXING		
		selected = false;
	}

	ImGui::End();
}

void ModuleScene::ShowImguiStatus() {
	ImGui::SetNextWindowPos(ImVec2(App->window->GetWidth()-300, 20));
	ImGui::SetNextWindowSize(ImVec2(300, 500));
	ImGui::Begin("Scene Manager");
	if (ImGui::CollapsingHeader("GameObjects"))
	{
		for (int i = 0; i < sceneObjects_.size(); i++)
		{
			if (sceneObjects_[i]->isSelected)
			{
				ComponentTransform *ct = (ComponentTransform*)sceneObjects_[i]->GetComponent(TRANSFORMATION);
				if (ct != nullptr)
				{
					ct->OnEditor(sceneObjects_[i]);
					ct->Update(sceneObjects_[i]);

				}

				ComponentMesh *cm = (ComponentMesh*)sceneObjects_[i]->GetComponent(MESH);
				if (cm != nullptr)
				{
					cm->OnEditor();
				}

				ComponentMaterial *cmat = (ComponentMaterial*)sceneObjects_[i]->GetComponent(MATERIAL);
				if (cmat != nullptr)
				{
					cmat->OnEditor();
				}
			}

		}
	}
	if (ImGui::CollapsingHeader("Settings"))
	{
		App->window->WindowImGui();
		App->renderer->ConfigurationManager();
	}
	//TODO: COLOR PICKER FOR AMBIENT LIGHT
	ImGui::End();

}

void ModuleScene::ImGuiMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New"))
			{
				if (ImGui::Button("Game Object"))
			
					ImGui::OpenPopup("New Game Object");
				
				if (ImGui::BeginPopupModal("New Game Object", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
				
					GameObject *object = new GameObject();
					static bool cm;
					static bool cam;
					static bool cmaterial;
					float pos[3] = {0,0,0};
					ImGui::Text("Hey there, you are creating an object");
					ImGui::Separator();
					ImGui::Text("What components do you want to add to your game object?");
					if (ImGui::Checkbox("Component mesh", &cm));
					if (ImGui::Checkbox("Component camera", &cam));
					if (ImGui::Checkbox("Component material", &cmaterial));
					if (ImGui::InputFloat3("Position (Comming soon...)", (float*)pos, 2));
					if (ImGui::Button("Create", ImVec2(120, 0)))
					{ 
						ImGui::CloseCurrentPopup();
						CreateGameObject(object, cm, cam);
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
					
					ImGui::EndPopup();
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Open"))
			{
				ImGui::MenuItem("Empty");

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Save"))
			{
				ImGui::MenuItem("Empty");

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Load"))
			{
				ImGui::MenuItem("Empty");

				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Exit"))
			{
				imguiFlag = SDL_SCANCODE_ESCAPE;
			}

			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Options"))
		{

		}
		ImGui::EndMainMenuBar();
	}
}

void ModuleScene::CreateGameObject(GameObject* obj, bool boolcm, bool boolcam)
{
	if (boolcm)
	{
		ComponentMesh* CM = new ComponentMesh(CUBE);
		obj->AddComponent(CM);
	}
	if (boolcam)
	{
		ComponentCamera* CAM = new ComponentCamera();
		obj->AddComponent(CAM);
	}
	
	ComponentTransform *ct = new ComponentTransform(float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f), Quat::identity);
	obj->AddComponent(ct);

	ComponentMaterial *cmat = new ComponentMaterial(obj);
	obj->AddComponent(cmat);

	sceneObjects_.push_back(obj);
}

/*	if (ImGui::TreeNode((void*)(intptr_t)i, "Game Object %d", i + 1))
{
for (int i = 0; i < sceneObjects_[i]->GetChilds().size(); i++)
{
if (ImGui::TreeNode((void*)(intptr_t)i, "Child %d", i + 1));
}
ImGui::TreePop();
}*/

void ModuleScene::ToggleFrustumAcceleration()
{
	accelerateFrustumCulling != accelerateFrustumCulling;
}


void ModuleScene::CreateRay(float2 screenPoint)
{
	std::map<float, GameObject*> objectsByDistance;
	float2 normalizedPoint = actualCamera->GetFrustum()->ScreenToViewportSpace(screenPoint, SCREEN_WIDTH, SCREEN_HEIGHT);
	//LineSegment ls = actualCamera->GetFrustum()->UnProjectLineSegment(normalizedPoint.x, normalizedPoint.y);
	LineSegment ls = actualCamera->GetFrustum()->UnProjectLineSegment(normalizedPoint.x, normalizedPoint.y);
	Ray ray = Ray(ls);
	LOG("Entered click and casted ray");
	std::vector<GameObject*> intersections;
	std::vector<GameObject*> objectlist;
	if (accelerateFrustumCulling) quadtree->Intersect(objectlist, *(actualCamera->GetFrustum()));
	else objectlist = sceneObjects_;
	float dist = 25000.0f;
	std::map<float, GameObject*>::iterator it = objectsByDistance.begin();
	// Check AABB's ONLY
	for (int i = 0; i < objectlist.size(); ++i)
	{
		ComponentMesh* cm = (ComponentMesh*)objectlist[i]->GetComponent(MESH);
		ComponentTransform* ct = (ComponentTransform*)objectlist[i]->GetComponent(TRANSFORMATION);
		if (cm != nullptr && ct != nullptr) {
			AABB newBox = *(cm->GetBoundingBox());
			newBox.TransformAsAABB(ct->GetGlobalTransform());
			if (ray.Intersects(newBox)) {
				Ray aux = ray;
				aux.Transform(ct->GetGlobalTransform().Inverted());
				if (objectlist[i]->CheckRayIntersection(aux, dist)) {
					objectsByDistance.insert(it, std::pair<float,GameObject*>(dist, objectlist[i]));
					LOG("Ray intersected with object %i",objectlist[i]->GetId());
				}
			}
		}
	}
	if (!objectsByDistance.empty()) {
		it = objectsByDistance.begin();
		LOG("Nearest object is %i \n", it->second->GetId());
	}
	//Get the triangle with the lowest distance, maps are ordered by the key.
}
