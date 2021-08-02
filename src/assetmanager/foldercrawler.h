#pragma once

#include "logger/logger.h"
#include <fstream>
#include <vector>
#include <Windows.h>
#include <string>

struct FileNode
{
  enum Type
  {
    None,
    Folder,
    File
  };

  Type m_Type;
  std::string m_Name;
};

class FolderCrawler
{
public:
  inline void Crawl(std::string path)
  {
    CrawlFolder(path, 0);
  }

private:
  void CrawlFolder(std::string path, int level)
  {
    std::vector<FileNode> files;
    ListCurrentFolder(path + "\\*", files);
    for (unsigned int i = 0; i < files.size(); i++)
    {
      switch (files[i].m_Type)
      {
      case FileNode::File: 
        ProcessFile(path + "\\" + files[i].m_Name, level + 1); 
        break;
      case FileNode::Folder: 
        ProcessFolder(path + "\\" + files[i].m_Name, level + 1);
        CrawlFolder(path + "\\" + files[i].m_Name, level + 1);
        break;
      }
    }
  }

  void ListCurrentFolder(std::string path, std::vector<FileNode>& files)
  {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    hFind = FindFirstFile(path.c_str(), &findFileData);
    while (hFind != INVALID_HANDLE_VALUE)
    {
      if (std::string(findFileData.cFileName) != "." && std::string(findFileData.cFileName) != "..")
      {
        FileNode Node = {
          findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? FileNode::Folder : FileNode::File,
          findFileData.cFileName
        };
        files.push_back(Node);
      }

      if (!FindNextFile(hFind, &findFileData))
      {
        hFind = INVALID_HANDLE_VALUE;
      }
    }
  }

  virtual void ProcessFile(std::string name, int level) {};
  virtual void ProcessFolder(std::string name, int level) {};
};