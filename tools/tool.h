#pragma once

#define TOOL

#include <filesystem>
namespace fs = std::filesystem;

#include "core/core.h"

// Tool entry point
extern int32_t ToolMain();

// Subcommand
class Subcommand
{
  public:
    void Register(CLI::App* command)
    {
        command->callback([this]() { this->Run(); });
    }
    virtual void Run() = 0;
};
