#include "ModuleWindow.h"
#include "SDL\include\SDL.h"


ModuleWindow::ModuleWindow()
{
}


ModuleWindow::~ModuleWindow()
{
}

bool ModuleWindow::Init() {
	bool ret = true;
	LOG("Init SDL window & surface");

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		//Try to use | SDL_WINDOW_RESIZABLE at the flags part
		window_ = SDL_CreateWindow("YunitiTresDe", SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		if (window_ == nullptr)
		{
			LOG("Failed to create window! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		//screen_surface not needed yet
		//screen_surface_ = SDL_GetWindowSurface(window_);
	}

	return ret;
}

SDL_Window* ModuleWindow::GetWindow() const
{
	return window_;
}

bool ModuleWindow::Start() {
	return true;
}

update_status ModuleWindow::Update() {
	return UPDATE_CONTINUE;
}

bool ModuleWindow::CleanUp() {
	LOG("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if (window_ != nullptr)
	{
		SDL_DestroyWindow(window_);
	}
	//Quit SDL subsystems
	SDL_Quit();
	return true;
}