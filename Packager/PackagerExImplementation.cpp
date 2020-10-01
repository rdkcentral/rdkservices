/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



// JUNK
// JUNK
// JUNK
#ifdef DEBUG
  #define JUNK_SLEEP_MS(ms) { std::this_thread::sleep_for(std::chrono::milliseconds( (ms) )); }
#else
  #define JUNK_SLEEP_MS(ms)  // noop
#endif
// JUNK
// JUNK
// JUNK

#include <glib.h>

#include "time.h"
#include <locale>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include "utils.h"

#include <interfaces/IPackager.h>

#include "PackagerExUtils.h"

#include "PackagerImplementation.h"
#include "PackagerExImplementation.h"


#define TEMPFILE_PATTERN  "/opt/tmpApp%08d.%s"

#define MB_in_BYTES  1000000

const int64_t WPEFramework::Plugin::PackagerImplementation::STORE_BYTES_QUOTA = 10 * MB_in_BYTES;
const char*   WPEFramework::Plugin::PackagerImplementation::STORE_NAME        = "DACstorage";
const char*   WPEFramework::Plugin::PackagerImplementation::STORE_KEY         = "4d4680a1-b3b0-471c-968b-39495d2b1cc3";

using namespace std;

namespace WPEFramework {
namespace Plugin {

  void PackagerImplementation::InitPackageDB()
  {
   // RegisterAll();

    auto path = g_build_filename("/opt", "persistent", nullptr);

    if (!PackagerExUtils::fileExists(path))
    {
        g_mkdir_with_parents(path, 0745);
    }

    auto file = g_build_filename(path, STORE_NAME, nullptr);

    bool success = PackagerExUtils::initDB(file, STORE_KEY);

    //LOGINFO(" ... SQLite >> Init()  %s ", (success ? " OK" : " FAILED !"));

    if(success)
    {
#ifdef USE_THREAD_POOL
      PackagerExUtils::setupThreadQ(this); // start thread Q
#endif

//JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK
//JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK
//
//  TEST CODE...
//
#if 0
      PackageInfoEx *pkg = Core::Service<PackageInfoEx>::Create<PackageInfoEx>("myName", "myVersion", "myID");

      pkg->setName("Test Name");
      pkg->setBundlePath("/opt/foo/bar/myAppFolder");
      pkg->setVersion("1.2.3");
      pkg->setPkgId("TestApp0123456");
      pkg->setInstalled("Fri Jul 31 16:54:41 UTC 2020");
      pkg->setSizeInBytes(123456);
      pkg->setType("DAC");


LOGINFO("########## addPkgRow \n");
  PackagerExUtils::addPkgRow(pkg);

    pkg->Release();

LOGINFO("########## hasPkgRow('TestApp0123456') == %s\n",
      (PackagerExUtils::hasPkgRow( "TestApp0123456" ) ? "TRUE" : "FALSE") );



LOGINFO("########## hasPkgRow('foo') == %s\n\n",
      (PackagerExUtils::hasPkgRow( "foo" ) ? "TRUE" : "FALSE") );


LOGINFO("########## showTable \n\n");
PackagerExUtils::showTable();


LOGINFO("\n########## delPkgRow\n");
  PackagerExUtils::delPkgRow("TestApp0123456");


LOGINFO("########## NOW ? hasPkgRow('TestApp0123456') == %s\n",
      (PackagerExUtils::hasPkgRow( "TestApp0123456" ) ? "TRUE" : "FALSE") );

#endif //00
//JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK
//JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK JUNK


// fprintf(stderr, "\n########## getPkgRow\n");
// PackageInfoEx* pp = PackagerExUtils::getPkgRow("TestApp0123456");

// PackageInfoEx::printPkg(pp);

     // PackagerExUtils::setupThreadQ(); // start thread Q

      // PackagerExUtils::installURL("http://10.0.2.15/test.tgz");
    }

    g_free(path);
    g_free(file);
  }

 void PackagerImplementation::TermPackageDB()
 {
   PackagerExUtils::term();
 }

  // DAC Installer API
  uint32_t PackagerImplementation::Install(const string& pkgId, const string& type, const string& url,
                                           const string& token, const string& listener)
  {
    if(IsInstalled(pkgId))
    {
      LOGERR(" - %s ... ALREADY installed", pkgId.c_str());

      NotifyIntallStep(Exchange::IPackager::INSTALLED, 0, pkgId); // technically true

      return 9; // FAIL  //PackagerExUtils::DACrc_t::dac_FAIL;
    }

    Core::InterlockedIncrement(_taskNumber);

    JobMeta_t job( _taskNumber, pkgId, type, url, token, listener);

    auto func = [this, job]
    {
      return this->doInstall(job);
    };

#ifdef USE_THREAD_POOL
    PackagerExUtils::addJob( job );
#else
    // TODO:  This is a bit "fire & forget" ... should improve with Thread Pool.
    std::thread threadObj( func );

 //  threadObj.join();
   threadObj.detach();
#endif

    return _taskNumber;
  }

  uint32_t PackagerImplementation::doInstall(const JobMeta_t &job)
  {
    return doInstall(job.taskId,
                     job.pkgId,
                     job.type,
                     job.url,
                     job.token,
                     job.listener);
  }

  uint32_t PackagerImplementation::doInstall(
        uint32_t taskId,
        const string& pkgId, const string& type, const string& url,
        const string& token, const string& listener)
  {
    char download_name[PATH_MAX];
    char uuid_path[PATH_MAX];

    // Validate URL
    if(PackagerExUtils::validateURL(url.c_str()) != PackagerExUtils::DACrc_t::dac_OK )
    {
        LOGERR(" ... Invlaid download URL >> %s \n", url.c_str());

        NotifyIntallStep(Exchange::IPackager::DOWNLOAD_FAILED, 0, pkgId, -1);

        return 11; // FAIL  //PackagerExUtils::DACrc_t::dac_FAIL;
    }

    std::string install_url  = url;      // Default: No JSON manifest - just a .tgz
    std::string install_name = pkgId;    // dummy defaults
    std::string install_ver  = "1.2.3";  // dummy defaults

//    NotifyIntallStep(INSTALL_START);

    std::string ext = PackagerExUtils::fileExtension(install_url);

    snprintf(download_name, PATH_MAX, TEMPFILE_PATTERN, taskId, ext.c_str() );
    LOGINFO(" - ... Using TEMP = %s", download_name);

    // Is URL a JSON manifset ?
    //
    bool isJSON = PackagerExUtils::fileEndsWith(install_url.c_str(), "json");

    if(isJSON)
    {
        LOGINFO(" ... DOWNLOAD >>>  %s", install_url.c_str());

        // Download JSON manifest...
        //
        if(PackagerExUtils::downloadJSON(install_url.c_str(), download_name) != PackagerExUtils::DACrc_t::dac_OK)
        {
            LOGERR(" ... ERROR:  Failed to download JSON >> %s \n", install_url.c_str());

            NotifyIntallStep(Exchange::IPackager::DOWNLOAD_FAILED, 0, pkgId, -2);

            return 22; // FAIL  //PackagerExUtils::DACrc_t::dac_FAIL;
        }

        PackagerExUtils::fileRemove(download_name); // Cleanup JSON

        // Parse JSON for meta...
        install_url  = PackagerExUtils::mPackageCfg["install"].String(); // update install from URL
        install_name = PackagerExUtils::mPackageCfg["name"].String();
        install_ver  = PackagerExUtils::mPackageCfg["version"].String();

        // Update download temp name...
        std::string ext = PackagerExUtils::fileExtension(install_url);
        snprintf(download_name, PATH_MAX, TEMPFILE_PATTERN, taskId, ext.c_str() );

        // Check NOT empty/exist
        if(install_url.empty()  || install_url  == "null" ||
           install_name.empty() || install_name == "null" ||
           install_ver.empty()  || install_ver  == "null" )
        {
            LOGERR( " ... Missing keys in JSON >> %s \n", url.c_str());
            LOGINFO(" ... install_url: %s   install_name: %s  install_ver: %s\n",
                     install_url.c_str(), install_name.c_str(), install_ver.c_str());

            NotifyIntallStep(Exchange::IPackager::DOWNLOAD_FAILED, 0, pkgId, -3);

            return 33; // FAIL  //PackagerExUtils::DACrc_t::dac_FAIL;
        }
    }

    // Validate URL
    if(PackagerExUtils::validateURL(install_url.c_str()) != PackagerExUtils::DACrc_t::dac_OK )
    {
        LOGERR(" ... Invlaid URL >> %s \n", url.c_str());

        NotifyIntallStep(Exchange::IPackager::DOWNLOAD_FAILED, 0, pkgId, -4);

        return 44; // FAIL  //PackagerExUtils::DACrc_t::dac_FAIL;
    }

    if(isJSON == false)
    {
      LOGWARN(" ... WARN:  No JSON manifest - just a .tgz - using dummy fields >> %s \n", url.c_str());
      // TODO:  Find a JSON manifest within the .tgz ?
    }

    NotifyIntallStep(Exchange::IPackager::DOWNLOADING, taskId, pkgId);
JUNK_SLEEP_MS(200);

    // Download TGZ package...
    //
    LOGINFO(" ... DOWNLOAD >>>  %s\n", install_url.c_str());

    if(PackagerExUtils::downloadURL(install_url.c_str(), download_name) != PackagerExUtils::DACrc_t::dac_OK)
    {
      NotifyIntallStep(Exchange::IPackager::DOWNLOAD_FAILED, 0, pkgId, -5);
JUNK_SLEEP_MS(200);

      LOGERR(" ... DOWNLOAD (%s)>>>  FAILED\n", install_url.c_str());
      return 55; // FAIL
    }
    else
    {
        NotifyIntallStep(Exchange::IPackager::DOWNLOADED, taskId, pkgId);
 JUNK_SLEEP_MS(200);

        NotifyIntallStep(Exchange::IPackager::VERIFYING, taskId, pkgId);  // aka "onExtractCommence"
JUNK_SLEEP_MS(200);

        // Get UUID ...
        std::string uuid_str = PackagerExUtils::getGUID();

        // Create path ... APPS_ROOT_PATH / {UUID} / {app}
        snprintf(uuid_path, PATH_MAX, "%s/%s/", APPS_ROOT_PATH, uuid_str.c_str());

        // EXTRACT
        //
        if(PackagerExUtils::extractPKG(download_name, uuid_path) != PackagerExUtils::DACrc_t::dac_OK)
        {
          // Clean up failed extraction
          //
          LOGERR(" ... EXTRACT >>>  FAILED\n");

          NotifyIntallStep(Exchange::IPackager::EXTRACTION_FAILED, taskId, pkgId, -1);

          PackagerExUtils::fileRemove(download_name); // Always cleanup
          PackagerExUtils::removeFolder(uuid_path);   // Remove debris

          return 66; // FAIL
        }

        NotifyIntallStep(Exchange::IPackager::VERIFIED, taskId, pkgId);
JUNK_SLEEP_MS(200);

        // TODO: look for JSON meta in app bundle...
        //
        LOGINFO(" ... INSTALLED >>> [ %s ]\n", install_name.c_str());

        NotifyIntallStep(Exchange::IPackager::INSTALLING, taskId, pkgId);
JUNK_SLEEP_MS(200);

        // INSTALL
        //
        PackagerExUtils::fileRemove(download_name); // Always cleanup

        NotifyIntallStep(Exchange::IPackager::INSTALLING, taskId, pkgId);
JUNK_SLEEP_MS(200);

        time_t rawtime;
        time(&rawtime);
        std::string strtime = ctime(&rawtime); 

        strtime.pop_back();  // NOTE:  Remove trailing '\n' >> illegal in JSON

        uint64_t bytes = PackagerExUtils::folderSize(uuid_path);

        PackageInfoEx* pkg = Core::Service<PackageInfoEx>::Create<PackageInfoEx>();

        pkg->setPkgId(pkgId);
        pkg->setName(install_name);
        pkg->setBundlePath(uuid_path);
        pkg->setVersion(install_ver);
        pkg->setInstalled( strtime );
        pkg->setSizeInBytes(bytes);
        pkg->setType(type);

        PackagerExUtils::addPkgRow(pkg); // add to SQL

        pkg->Release();

JUNK_SLEEP_MS(200);

        NotifyIntallStep(Exchange::IPackager::INSTALLED, taskId, pkgId);
        LOGINFO(" ... COMPLETE (%s) --------------------------------------------------------\n\n\n", install_url.c_str());
    }

    return 0; // no error
  }

  uint32_t PackagerImplementation::Remove( const string& pkgId, const string& listener)
  {
    LOGINFO("... Remove(%s, %s) - ENTER ", pkgId.c_str(), listener.c_str());

    PackageInfoEx* pkg = PackagerExUtils::getPkgRow(pkgId);

    if(pkg)
    {
      LOGINFO(" removeFolder( %s ) ... found", pkg->BundlePath().c_str());

      PackagerExUtils::removeFolder(pkg->BundlePath());

      bool rc = PackagerExUtils::delPkgRow(pkgId);

      if(rc == false)
      {
        LOGINFO("... Remove(%s, %s) - FAILED... not found ? ", pkgId.c_str(), listener.c_str());

        NotifyIntallStep(Exchange::IPackager::REMOVE_FAILED, 0, pkgId, -1);

        return -1; // FAILED
      }
    }
    else
    {
      LOGINFO(" .Remove( %s ) ... Found", pkgId.c_str());
    }

    return 0; // SUCCESS
  }

  uint32_t PackagerImplementation::Cancel( const string& task, const string& listener)
  {
    DDD();

    // TODO:
    // fprintf(stderr, "\nHUGH >>>>> Fill LIST >>> mPPPlist.size()  = %ld ", mPPPlist.size() );

    return 0;
  }

  uint32_t PackagerImplementation::IsInstalled(const string& pkgId)//, JsonObject &response)
  {
    // LOGINFO("\n\nPackagerExImplementation::IsInstalled() ... pkgId: [%s]\n\n", pkgId.c_str() );

    return PackagerExUtils::hasPkgRow( pkgId );
  }

  uint32_t PackagerImplementation::GetInstallProgress(const string& task)
  {
    DDD();

   NotifyIntallStep(Exchange::IPackager::state::IDLE);

    // TODO:
    return 42;
  }

  PackageInfoEx::IIterator* PackagerImplementation::GetInstalled()
  {
    PackagerExUtils::updatePkgList( _packageList ); // Populate 'list' from SQL

    PackageInfoExIterator *iter = Core::Service<PackageInfoExIterator>::Create<PackageInfoExIterator>(_packageList);

    return iter;
  }

  PackageInfoEx* PackagerImplementation::GetPackageInfo(const string& pkgId)
  {
    LOGINFO("DEBUG:  GetPackageInfo() - ENTER" );

    PackageInfoEx* pkg = PackagerExUtils::getPkgRow(pkgId);

    // if(pkg)
    // {
    //   PackagerExUtils::showTable();
    // }

    return pkg;
  }

  int64_t PackagerImplementation::GetAvailableSpace()
  {
    int64_t used_bytes = PackagerExUtils::sumSizeInBytes();

    // LOGINFO("PackagerExImplementation::GetAvailableSpace()  ... used_bytes: %jd ", used_bytes);
    // LOGINFO("PackagerExImplementation::GetAvailableSpace()  ... STORE_BYTES_QUOTA: %jd ", STORE_BYTES_QUOTA);
    // LOGINFO("PackagerExImplementation::GetAvailableSpace()  ... bytes_left: %jd ", (STORE_BYTES_QUOTA - used_bytes));

    return ((STORE_BYTES_QUOTA - used_bytes)/1000); // in KB
  }
  }  // namespace Plugin
}  // namespace WPEFramework