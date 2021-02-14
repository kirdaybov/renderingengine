#include "app/application.h"
#include "assetmanager/assetmanager.h"

//int main(size_t argc, char** argv)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
  gAssetManager.LoadAssets();
  gApplicationInstanceManager.StartNewInstance();
  return 0;
}