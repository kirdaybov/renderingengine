#include "app/application.h"
#include "assetmanager/assetmanager.h"

int main(size_t argc, char** argv)
{
  gAssetManager.LoadAssets();
  gApplicationInstanceManager.StartNewInstance();
  return 0;
}