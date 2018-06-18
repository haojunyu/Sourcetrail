#include "settings/Settings.h"

#include "utility/logging/logging.h"
#include "utility/text/TextAccess.h"
#include "utility/utility.h"

Settings::Settings(const Settings& other)
	: m_config(other.m_config->createCopy())
	, m_filePath(other.m_filePath)
{
}

Settings& Settings::operator=(const Settings& other)
{
	if (&other != this)
	{
		m_filePath = other.m_filePath;
		m_config = other.m_config->createCopy();
	}

	return *this;
}

Settings::~Settings()
{
}

bool Settings::load(const FilePath& filePath, bool readOnly)
{
	m_readOnly = readOnly;

	if (filePath.exists())
	{
		m_config = ConfigManager::createAndLoad(TextAccess::createFromFile(filePath));
		m_filePath = filePath;
		return true;
	}
	else
	{
		clear();
		m_filePath = filePath;
		LOG_WARNING(L"File for Settings not found: " + filePath.wstr());
		return false;
	}
}

void Settings::save()
{
	if (m_readOnly)
	{
		return;
	}

	if (m_config.get() && !m_filePath.empty())
	{
		m_config->save(m_filePath.str());
	}
	else
	{
		LOG_WARNING(L"Settings were not saved: " + m_filePath.wstr());
	}
}

void Settings::save(const FilePath& filePath)
{
	setFilePath(filePath);

	save();
}

void Settings::clear()
{
	m_config = ConfigManager::createEmpty();
	m_filePath = FilePath();
}

const FilePath& Settings::getFilePath() const
{
	return m_filePath;
}

size_t Settings::getVersion() const
{
	return getValue<int>("version", 0);
}

void Settings::setVersion(size_t version)
{
	setValue<int>("version", version);
}

FilePath Settings::expandPath(const FilePath& path)
{
	std::vector<FilePath> paths = path.expandEnvironmentVariables();
	if (!paths.empty())
	{
		if (paths.size() > 1)
		{
			LOG_WARNING(
				L"Environment variable in path \"" + path.wstr() + L"\" has been expanded to " + std::to_wstring(paths.size()) +
				L"paths, but only \"" + paths.front().wstr() + L"\" will be used."
			);
		}
		return paths.front();
	}
	return FilePath();
}

std::vector<FilePath> Settings::expandPaths(const std::vector<FilePath>& paths)
{
	std::vector<FilePath> expanedPaths;
	for (const FilePath& path : paths)
	{
		utility::append(expanedPaths, path.expandEnvironmentVariables());
	}
	return expanedPaths;
}

Settings::Settings()
{
	clear();
}

void Settings::setFilePath(const FilePath& filePath)
{
	m_filePath = filePath;
}

std::vector<FilePath> Settings::getPathValues(const std::string& key) const
{
	std::vector<FilePath> paths;
	for (const std::wstring& value : getValues<std::wstring>(key, {}))
	{
		paths.push_back(FilePath(value));
	}
	return paths;
}

bool Settings::setPathValues(const std::string& key, const std::vector<FilePath>& paths)
{
	std::vector<std::wstring> values;
	for (const FilePath& path : paths)
	{
		values.push_back(path.wstr());
	}

	return setValues(key, values);
}

bool Settings::isValueDefined(const std::string& key) const
{
	return m_config->isValueDefined(key);
}

void Settings::removeValues(const std::string& key)
{
	m_config->removeValues(key);
}

void Settings::enableWarnings() const
{
	m_config->setWarnOnEmptyKey(true);
}

void Settings::disableWarnings() const
{
	m_config->setWarnOnEmptyKey(false);
}
