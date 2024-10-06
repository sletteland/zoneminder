//
// ZoneMinder Monitor::ZlMediaKitManager Class Implementation, $Date$, $Revision$
// Copyright (C) 2022 Jonathan Bennett
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "zm_crypt.h"
#include "zm_monitor.h"
#include "zm_server.h"
#include "zm_time.h"

#include <algorithm>
#include <regex>

std::string remove_newlines(std::string input);
std::string escape_json_string(std::string input);

Monitor::ZlMediaKitManager::ZlMediaKitManager(Monitor *parent_) :
  parent(parent_),
  ZlMediaKit_Healthy(false) {
  Use_RTSP_Restream = false;
    Warning(" ZlMediaKitManager --> add stream");
  Warning(" config.zlmediakit_path = %s ",   config.zlmediakit_path  );
  Warning(" config.zlmediakit_secret = %s ",   config.zlmediakit_secret  );
  if ((config.zlmediakit_path != nullptr) && (config.zlmediakit_path[0] != '\0')) {
    ZlMediaKit_endpoint = config.zlmediakit_path;
    Warning(" config.zlmediakit_path = %s ",   config.zlmediakit_path  );
    Warning(" ZlMediaKit_endpoint = %s ",   ZlMediaKit_endpoint.c_str()  );
    //remove the trailing slash if present
    if (ZlMediaKit_endpoint.back() == '/') ZlMediaKit_endpoint.pop_back();
  } else {
    ZlMediaKit_endpoint = "http://127.0.0.1";
  }
  if ((config.zlmediakit_secret != nullptr) && (config.zlmediakit_secret[0] != '\0')) {
    ZlMediaKit_secret = config.zlmediakit_secret;
  }
  else {
   ZlMediaKit_secret = "no secret" ;
  }

  Warning(" config.zlmediakit_path = %s ",   config.zlmediakit_path  );
  Warning(" config.zlmediakit_secret = %s ",   config.zlmediakit_secret  );
  Warning(" config.janus_secret = %s ",   config.janus_secret  );
  Warning(" config.rtsp2web_path = %s ",   config.rtsp2web_path  );
  Warning("Monitor %u rtsp url is %s", parent->id, rtsp_path.c_str());

  rtsp_path = parent->path;
  if (!parent->user.empty()) {
    rtsp_username = escape_json_string(parent->user);
    rtsp_password = escape_json_string(parent->pass);
    if (rtsp_path.find("rtsp://") == 0) {
      rtsp_path = "rtsp://" + rtsp_username + ":" + rtsp_password + "@" + rtsp_path.substr(7, std::string::npos);
    } else {
      rtsp_path = "rtsp://" + rtsp_username + ":" + rtsp_password + "@" + rtsp_path;
    }
  }
  Debug(1, "Monitor %u rtsp url is %s", parent->id, rtsp_path.c_str());
}

Monitor::ZlMediaKitManager::~ZlMediaKitManager() {
  remove_from_ZlMediaKit();
}

int Monitor::ZlMediaKitManager::check_ZlMediaKit() {
  curl = curl_easy_init();
  if (!curl) {
    Error("Failed to init curl");
    return -1;
  }

  //Assemble our actual request
  std::string response;
  std::string endpoint = ZlMediaKit_endpoint+"/index/api/isMediaOnline?secret="+ZlMediaKit_secret+"&schema=rtsp&vhost=__defaultVhost__&app=live&stream="+std::to_string(parent->id);
  Warning("Monitor ID =  %u ", parent->id );
  curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
  //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
    Warning("Attempted to send to %s and got %s", endpoint.c_str(), curl_easy_strerror(res));

  if (res != CURLE_OK) {
    Warning("Attempted to send to %s and got %s", endpoint.c_str(), curl_easy_strerror(res));
    return -1;
  }

  response = remove_newlines(response);
  Warning("Queried for stream status: %s", response.c_str());
  if (response.find("\"online\": 0") != std::string::npos) {
    if (response.find("stream not found") != std::string::npos) {
      Debug(1, "Mountpoint Missing");
      return 0;
    } else {
      Warning("unknown ZlMediaKit error");
      return 0;
    }
  }

  return 1;
}

int Monitor::ZlMediaKitManager::add_to_ZlMediaKit() {
std::string endpoint ;
  Debug(1, "Adding stream to ZlMediaKit");
  curl = curl_easy_init();
  if (!curl) {
    Error("Failed to init curl");
    return -1;
  }
if (parent->ZlMediaKit_audio_enabled)
{
// with sound 
endpoint = ZlMediaKit_endpoint+"/index/api/addFFmpegSource?secret="+ZlMediaKit_secret+"&src_url="+rtsp_path+"&dst_url=rtsp://localhost/live/"+std::to_string(parent->id)+"&enable_rtsp=true&enable_rtmp=false&enable_ts=false&enable_fmp4=false&hls_demand=false&rtsp_demand=false&rtmp_demand=false&ts_demand=false&fmp4_demand=false&mp4_as_player=false&timeout_ms=10000&ffmpeg_cmd_key=ffmpeg.cmd";
}
else
{
// without sound
endpoint = ZlMediaKit_endpoint+"/index/api/addStreamProxy?secret="+ZlMediaKit_secret+"&vhost=__defaultVhost__&app=live&stream="+std::to_string(parent->id)+"&url="+rtsp_path+"&retry_count=-1&rtp_type=0&timeout_sec=5&enable_hls=false&enable_hls_fmp4=false&enable_mp4=false&enable_rtsp=true&enable_rtmp=false&enable_ts=false&enable_fmp4=false&hls_demand=false&rtsp_demand=false&rtmp_demand=false&ts_demand=false&fmp4_demand=false&enable_audio=false&add_mute_audio=false&mp4_max_second=10&mp4_as_player=false&auto_close=false";
}

  //Assemble our actual request
//  postData += rtsp_path;
//  postData += "\", \"on_demand\": true, \"debug\": false, \"status\": 0}}}";

  Warning( "Sending  %s", endpoint.c_str());

  CURLcode res;
  std::string response;

  curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    Error("Failed to curl_easy_perform adding rtsp stream %s", curl_easy_strerror(res));
    return -1;
  }

  response = remove_newlines(response);
  Warning( "Adding stream response: %s", response.c_str());
  //scan for missing session or handle id "No such session" "no such handle"
  if (response.find("\"code\" : 0") == std::string::npos) {
    if (response == "{    \"code\": 1,    \"payload\": \"stream already exists\"}") {
      Warning("ZlMediaKit failed adding stream, response: %s ->return (0)", response.c_str());
    } else {
      Warning("ZlMediaKit failed adding stream, response: %s ->return(-2)", response.c_str());
     return -2;
    }
  } else {
    Warning( "Added stream to ZlMediaKit: %s", response.c_str());
  }

  return 0;
}

int Monitor::ZlMediaKitManager::remove_from_ZlMediaKit() {
  curl = curl_easy_init();
  if (!curl) return -1;

  std::string endpoint = ZlMediaKit_endpoint+"/index/api/close_streams?secret="+ZlMediaKit_secret+"&schema=rtsp&vhost=__defaultVhost__&app=live&stream="+std::to_string(parent->id)+"&force=1";
  std::string response;

  curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    Warning("Libcurl attempted %s got %s", endpoint.c_str(), curl_easy_strerror(res));
  } else {
     Warning( "Removed stream from ZlMediaKit: %s", remove_newlines(response).c_str());
  }

  curl_easy_cleanup(curl);
  return 0;
}

size_t Monitor::ZlMediaKitManager::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

/* already defined in monitor_rtsp2web.cpp....
  std::string remove_newlines( std::string str ) {
  while (!str.empty() && str.find("\n") != std::string::npos)
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.cend());
  return str;
}
*/

/*
std::string escape_json_string( std::string input ) {
  std::string tmp;
  tmp = regex_replace(input, std::regex("\n"), "\\n");
  tmp = regex_replace(tmp,   std::regex("\b"), "\\b");
  tmp = regex_replace(tmp,   std::regex("\f"), "\\f");
  tmp = regex_replace(tmp,   std::regex("\r"), "\\r");
  tmp = regex_replace(tmp,   std::regex("\t"), "\\t");
  tmp = regex_replace(tmp,   std::regex("\""), "\\\"");
  tmp = regex_replace(tmp,   std::regex("[\\\\]"), "\\\\");
  return tmp;
}
*/
