#include "AIManager.h"
#include "EditorErrorReporter.h"
#include "OpenAIProvider.h"
#include "LlamaProvider.h"

#include <algorithm>
#include <stdexcept>
#include <filesystem>

namespace ai_editor {

AIManager::AIManager()
    : nextCallbackId_(0), activeProvider_(nullptr),
      templateManager_(std::make_shared<PromptTemplateManager>())
{
    // Register built-in providers
    registerProvider("openai", [this](const std::map<std::string, std::string>& options) {
        ProviderOptions providerOpts;
        for (const auto& [key, value] : options) {
            providerOpts.additionalOptions[key] = value;
        }
        return createOpenAIProvider(providerOpts);
    });
    
    registerProvider("llama", [this](const std::map<std::string, std::string>& options) {
        ProviderOptions providerOpts;
        for (const auto& [key, value] : options) {
            providerOpts.additionalOptions[key] = value;
        }
        return createLlamaProvider(providerOpts);
    });
}

AIManager::~AIManager()
{
    // Clean up providers
    for (auto& [id, provider] : providers_) {
        if (provider) {
            try {
                // Call any cleanup needed
            } catch (const std::exception& e) {
                EditorErrorReporter::reportError(
                    "AIManager",
                    "Exception during provider cleanup: " + std::string(e.what()),
                    "Ignoring and continuing shutdown"
                );
            }
        }
    }
    
    providers_.clear();
    activeProvider_ = nullptr;
}

bool AIManager::initialize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Register the OpenAI provider factory with the AIProviderFactory
        registerOpenAIProvider();
        
        // Register the LLama provider factory with the AIProviderFactory
        registerLlamaProvider();
        
        return true;
    } catch (const std::exception& e) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Failed to initialize: " + std::string(e.what()),
            "Check provider initialization"
        );
        return false;
    }
}

bool AIManager::registerProvider(const std::string& providerType, const ProviderOptions& options)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Convert provider type to lowercase for case-insensitive comparison
        std::string providerTypeLower = providerType;
        std::transform(providerTypeLower.begin(), providerTypeLower.end(), providerTypeLower.begin(), 
                       [](unsigned char c) { return std::tolower(c); });
        
        // Check if this provider is already registered
        if (providers_.find(providerTypeLower) != providers_.end()) {
            EditorErrorReporter::reportWarning(
                "AIManager",
                "Provider already registered: " + providerType,
                "Use setProviderOptions to update options"
            );
            return false;
        }
        
        // Create the provider instance
        auto provider = AIProviderFactory::createProvider(providerTypeLower, options);
        if (!provider) {
            EditorErrorReporter::reportError(
                "AIManager",
                "Failed to create provider: " + providerType,
                "Check provider type and options"
            );
            return false;
        }
        
        // Add the provider to the map
        providers_[providerTypeLower] = std::move(provider);
        
        // If this is the first provider, set it as active
        if (activeProviderType_.empty()) {
            activeProviderType_ = providerTypeLower;
            notifyProviderChangeCallbacks(providerTypeLower);
        }
        
        return true;
    } catch (const std::exception& e) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Exception registering provider: " + std::string(e.what()),
            "Check provider initialization"
        );
        return false;
    }
}

std::vector<std::string> AIManager::getRegisteredProviderTypes() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> types;
    types.reserve(providers_.size());
    
    for (const auto& [type, _] : providers_) {
        types.push_back(type);
    }
    
    return types;
}

bool AIManager::setActiveProvider(const std::string& providerType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Convert provider type to lowercase for case-insensitive comparison
    std::string providerTypeLower = providerType;
    std::transform(providerTypeLower.begin(), providerTypeLower.end(), providerTypeLower.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    // Check if the provider is registered
    if (providers_.find(providerTypeLower) == providers_.end()) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Provider not registered: " + providerType,
            "Register the provider first"
        );
        return false;
    }
    
    // Set the active provider
    activeProviderType_ = providerTypeLower;
    
    // Notify callbacks
    notifyProviderChangeCallbacks(providerTypeLower);
    
    return true;
}

std::string AIManager::getActiveProviderType() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return activeProviderType_;
}

bool AIManager::isProviderRegistered(const std::string& providerType) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Convert provider type to lowercase for case-insensitive comparison
    std::string providerTypeLower = providerType;
    std::transform(providerTypeLower.begin(), providerTypeLower.end(), providerTypeLower.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    return providers_.find(providerTypeLower) != providers_.end();
}

std::vector<ModelInfo> AIManager::listAvailableModels()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "No active provider",
            "Set an active provider first"
        );
        return {};
    }
    
    return provider->listAvailableModels();
}

std::vector<ModelInfo> AIManager::listAvailableModels(const std::string& providerType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getProvider(providerType);
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Provider not registered: " + providerType,
            "Register the provider first"
        );
        return {};
    }
    
    return provider->listAvailableModels();
}

ModelInfo AIManager::getCurrentModelInfo() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "No active provider",
            "Set an active provider first"
        );
        return {};
    }
    
    return provider->getCurrentModelInfo();
}

bool AIManager::setCurrentModel(const std::string& modelId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "No active provider",
            "Set an active provider first"
        );
        return false;
    }
    
    bool result = provider->setCurrentModel(modelId);
    if (result) {
        notifyModelChangeCallbacks(activeProviderType_, modelId);
    }
    
    return result;
}

bool AIManager::setCurrentModel(const std::string& providerType, const std::string& modelId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getProvider(providerType);
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Provider not registered: " + providerType,
            "Register the provider first"
        );
        return false;
    }
    
    bool result = provider->setCurrentModel(modelId);
    if (result) {
        notifyModelChangeCallbacks(providerType, modelId);
    }
    
    return result;
}

CompletionResponse AIManager::sendCompletionRequest(
    const std::vector<Message>& messages,
    const std::vector<ToolDefinition>& tools)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Cannot send completion request: No active provider",
            "Set an active provider first"
        );
        return {CompletionResponse::Status::API_ERROR, "", {}, "No active provider", {}};
    }
    
    return provider->sendCompletionRequest(messages, tools);
}

std::vector<float> AIManager::generateEmbedding(
    const std::string& input,
    const std::optional<std::string>& modelId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "No active provider",
            "Set an active provider first"
        );
        return {};
    }
    
    return provider->generateEmbedding(input, modelId);
}

ProviderOptions AIManager::getProviderOptions() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "No active provider",
            "Set an active provider first"
        );
        return {};
    }
    
    return provider->getOptions();
}

ProviderOptions AIManager::getProviderOptions(const std::string& providerType) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getProvider(providerType);
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Provider not registered: " + providerType,
            "Register the provider first"
        );
        return {};
    }
    
    return provider->getOptions();
}

bool AIManager::setProviderOptions(const ProviderOptions& options)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "No active provider",
            "Set an active provider first"
        );
        return false;
    }
    
    provider->setOptions(options);
    return true;
}

bool AIManager::setProviderOptions(const std::string& providerType, const ProviderOptions& options)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getProvider(providerType);
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Provider not registered: " + providerType,
            "Register the provider first"
        );
        return false;
    }
    
    provider->setOptions(options);
    return true;
}

bool AIManager::supportsCapability(const std::string& capability) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        return false;
    }
    
    return provider->supportsCapability(capability);
}

bool AIManager::supportsCapability(const std::string& providerType, const std::string& capability) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getProvider(providerType);
    if (!provider) {
        return false;
    }
    
    return provider->supportsCapability(capability);
}

int AIManager::registerProviderChangeCallback(std::function<void(const std::string&)> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!callback) {
        return -1;
    }
    
    int callbackId = nextCallbackId_++;
    providerChangeCallbacks_[callbackId] = std::move(callback);
    
    return callbackId;
}

void AIManager::unregisterProviderChangeCallback(int callbackId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = providerChangeCallbacks_.find(callbackId);
    if (it != providerChangeCallbacks_.end()) {
        providerChangeCallbacks_.erase(it);
    }
}

int AIManager::registerModelChangeCallback(std::function<void(const std::string&, const std::string&)> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!callback) {
        return -1;
    }
    
    int callbackId = nextCallbackId_++;
    modelChangeCallbacks_[callbackId] = std::move(callback);
    
    return callbackId;
}

void AIManager::unregisterModelChangeCallback(int callbackId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = modelChangeCallbacks_.find(callbackId);
    if (it != modelChangeCallbacks_.end()) {
        modelChangeCallbacks_.erase(it);
    }
}

IAIProvider* AIManager::getActiveProvider() const
{
    if (activeProviderType_.empty()) {
        return nullptr;
    }
    
    auto it = providers_.find(activeProviderType_);
    if (it == providers_.end()) {
        return nullptr;
    }
    
    return it->second.get();
}

IAIProvider* AIManager::getProvider(const std::string& providerType) const
{
    // Convert provider type to lowercase for case-insensitive comparison
    std::string providerTypeLower = providerType;
    std::transform(providerTypeLower.begin(), providerTypeLower.end(), providerTypeLower.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    auto it = providers_.find(providerTypeLower);
    if (it == providers_.end()) {
        return nullptr;
    }
    
    return it->second.get();
}

void AIManager::notifyProviderChangeCallbacks(const std::string& providerType)
{
    // Make a copy of the callbacks to avoid deadlocks if callbacks modify the callback map
    auto callbacksCopy = providerChangeCallbacks_;
    
    // Unlock the mutex before calling callbacks
    mutex_.unlock();
    
    // Call each callback
    for (const auto& [_, callback] : callbacksCopy) {
        try {
            callback(providerType);
        } catch (const std::exception& e) {
            EditorErrorReporter::reportError(
                "AIManager",
                "Exception in provider change callback: " + std::string(e.what()),
                "Check callback implementation"
            );
        }
    }
    
    // Relock the mutex
    mutex_.lock();
}

void AIManager::notifyModelChangeCallbacks(const std::string& providerType, const std::string& modelId)
{
    // Make a copy of the callbacks to avoid deadlocks if callbacks modify the callback map
    auto callbacksCopy = modelChangeCallbacks_;
    
    // Unlock the mutex before calling callbacks
    mutex_.unlock();
    
    // Call each callback
    for (const auto& [_, callback] : callbacksCopy) {
        try {
            callback(providerType, modelId);
        } catch (const std::exception& e) {
            EditorErrorReporter::reportError(
                "AIManager",
                "Exception in model change callback: " + std::string(e.what()),
                "Check callback implementation"
            );
        }
    }
    
    // Relock the mutex
    mutex_.lock();
}

void AIManager::registerProvider(const std::string& type, ProviderCreatorFunc creator) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (providerCreators_.find(type) != providerCreators_.end()) {
        EditorErrorReporter::reportWarning(
            "AIManager",
            "Provider type already registered: " + type,
            "Overwriting existing registration"
        );
    }
    
    providerCreators_[type] = creator;
    EditorErrorReporter::reportInfo(
        "AIManager",
        "Registered provider type: " + type,
        ""
    );
}

std::shared_ptr<IAIProvider> AIManager::createProvider(
    const std::string& type,
    const std::map<std::string, std::string>& options) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = providerCreators_.find(type);
    if (it == providerCreators_.end()) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Unknown provider type: " + type,
            "Available types: " + getAvailableProviderTypes()
        );
        return nullptr;
    }
    
    try {
        return it->second(options);
    } catch (const std::exception& e) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Failed to create provider: " + type,
            "Error: " + std::string(e.what())
        );
        return nullptr;
    }
}

bool AIManager::initializeProvider(
    const std::string& type,
    const std::map<std::string, std::string>& options) 
{
    // Create the provider if it doesn't exist
    if (providers_.find(type) == providers_.end()) {
        auto provider = createProvider(type, options);
        if (!provider) {
            return false;
        }
        
        providers_[type] = provider;
    }
    
    auto& provider = providers_[type];
    
    // Initialize the provider
    try {
        if (!provider->initialize(options)) {
            EditorErrorReporter::reportError(
                "AIManager",
                "Failed to initialize provider: " + type,
                "Provider returned false from initialize()"
            );
            return false;
        }
        
        // Set as active provider if we don't have one
        if (!activeProvider_) {
            activeProvider_ = provider;
            
            // Notify of provider change
            for (const auto& callback : providerChangeCallbacks_) {
                callback(type);
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Exception initializing provider: " + type,
            "Error: " + std::string(e.what())
        );
        return false;
    }
}

std::string AIManager::getAvailableProviderTypes() const {
    std::string result;
    
    for (const auto& [type, _] : providerCreators_) {
        if (!result.empty()) {
            result += ", ";
        }
        result += type;
    }
    
    return result;
}

std::vector<std::string> AIManager::getAvailableProviderTypesList() const {
    std::vector<std::string> result;
    
    for (const auto& [type, _] : providerCreators_) {
        result.push_back(type);
    }
    
    return result;
}

std::vector<std::string> AIManager::getInitializedProviderTypesList() const {
    std::vector<std::string> result;
    
    for (const auto& [type, provider] : providers_) {
        if (provider && provider->isInitialized()) {
            result.push_back(type);
        }
    }
    
    return result;
}

bool AIManager::setActiveProvider(const std::string& type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = providers_.find(type);
    if (it == providers_.end()) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Cannot set active provider: Type not initialized: " + type,
            "Initialize the provider first"
        );
        return false;
    }
    
    if (!it->second->isInitialized()) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Cannot set active provider: Provider not initialized: " + type,
            "Ensure the provider is initialized first"
        );
        return false;
    }
    
    std::string previousType = activeProvider_ ? activeProvider_->getProviderName() : "";
    activeProvider_ = it->second;
    
    // Notify of provider change if it's actually different
    if (previousType != type) {
        for (const auto& callback : providerChangeCallbacks_) {
            callback(type);
        }
    }
    
    return true;
}

std::shared_ptr<IAIProvider> AIManager::getActiveProvider() const {
    return activeProvider_;
}

std::string AIManager::getActiveProviderType() const {
    if (!activeProvider_) {
        return "";
    }
    
    // Find the type by comparing provider instances
    for (const auto& [type, provider] : providers_) {
        if (provider == activeProvider_) {
            return type;
        }
    }
    
    return "";
}

void AIManager::addProviderChangeCallback(ProviderChangeCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    providerChangeCallbacks_.push_back(callback);
}

// Helper to initialize the default local LLama provider if a model path is provided
bool AIManager::initializeLocalLlamaProvider(const std::string& modelPath) {
    if (modelPath.empty()) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Cannot initialize LLama provider: Empty model path",
            "Provide a valid path to a LLama model file"
        );
        return false;
    }
    
    // Check if path exists
    if (!std::filesystem::exists(modelPath)) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Cannot initialize LLama provider: Model path does not exist: " + modelPath,
            "Provide a valid path to a LLama model file"
        );
        return false;
    }
    
    // Create options map
    std::map<std::string, std::string> options;
    options["model_path"] = modelPath;
    
    // Initialize the LLama provider
    return initializeProvider("llama", options);
}

// Template-related methods

std::shared_ptr<PromptTemplate> AIManager::getCurrentTemplate() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "No active provider",
            "Set an active provider first"
        );
        return nullptr;
    }
    
    return provider->getCurrentTemplate();
}

std::shared_ptr<PromptTemplate> AIManager::getCurrentTemplate(const std::string& providerType) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getProvider(providerType);
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Provider not registered: " + providerType,
            "Register the provider first"
        );
        return nullptr;
    }
    
    return provider->getCurrentTemplate();
}

bool AIManager::setCurrentTemplate(const std::string& templateId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "No active provider",
            "Set an active provider first"
        );
        return false;
    }
    
    bool result = provider->setCurrentTemplate(templateId);
    if (result) {
        notifyTemplateChange(templateId);
    }
    
    return result;
}

bool AIManager::setCurrentTemplate(const std::string& providerType, const std::string& templateId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getProvider(providerType);
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Provider not registered: " + providerType,
            "Register the provider first"
        );
        return false;
    }
    
    bool result = provider->setCurrentTemplate(templateId);
    if (result) {
        notifyTemplateChange(templateId);
    }
    
    return result;
}

std::vector<std::string> AIManager::getAvailableTemplates() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getActiveProvider();
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "No active provider",
            "Set an active provider first"
        );
        return {};
    }
    
    return provider->getAvailableTemplates();
}

std::vector<std::string> AIManager::getAvailableTemplates(const std::string& providerType) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto provider = getProvider(providerType);
    if (!provider) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Provider not registered: " + providerType,
            "Register the provider first"
        );
        return {};
    }
    
    return provider->getAvailableTemplates();
}

PromptTemplateInfo AIManager::getTemplateInfo(const std::string& templateId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto template_ = templateManager_->getTemplate(templateId);
    if (!template_) {
        EditorErrorReporter::reportError(
            "AIManager",
            "Template not found: " + templateId,
            "Check if the template ID is valid"
        );
        return {};
    }
    
    return template_->getInfo();
}

std::vector<PromptTemplateInfo> AIManager::getAllTemplateInfo() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto templates = templateManager_->getAllTemplates();
    std::vector<PromptTemplateInfo> templateInfos;
    templateInfos.reserve(templates.size());
    
    for (const auto& template_ : templates) {
        templateInfos.push_back(template_->getInfo());
    }
    
    return templateInfos;
}

int AIManager::addTemplateChangeCallback(TemplateChangeCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int callbackId = nextCallbackId_++;
    templateChangeCallbacks_[callbackId] = std::move(callback);
    return callbackId;
}

void AIManager::removeTemplateChangeCallback(int callbackId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    templateChangeCallbacks_.erase(callbackId);
}

void AIManager::notifyTemplateChange(const std::string& templateId) {
    // Make a copy of the callbacks to avoid holding the lock during callback execution
    std::map<int, TemplateChangeCallback> callbacks;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks = templateChangeCallbacks_;
    }
    
    // Call all callbacks
    for (const auto& [id, callback] : callbacks) {
        try {
            callback(templateId);
        } catch (const std::exception& e) {
            EditorErrorReporter::reportError(
                "AIManager",
                "Exception in template change callback: " + std::string(e.what()),
                "Check callback implementation"
            );
        }
    }
}

} // namespace ai_editor 