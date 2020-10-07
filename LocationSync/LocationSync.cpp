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
 
#include "LocationSync.h"

#include "utils.h"
#include "tzfile.h"

#define ZONEINFO_DIR "/usr/share/zoneinfo"

#define TIMEBUFSIZE 16

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(LocationSync, 1, 0);

    static Core::ProxyPoolType<Web::Response> responseFactory(4);
    static Core::ProxyPoolType<Web::JSONBodyType<LocationSync::Data>> jsonResponseFactory(4);

#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
    LocationSync::LocationSync()
        : _skipURL(0)
        , _source()
        , _sink(this)
        , _service(nullptr)
    {
        RegisterAll();
    }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif

    /* virtual */ LocationSync::~LocationSync()
    {
        UnregisterAll();
    }

    /* virtual */ const string LocationSync::Initialize(PluginHost::IShell* service)
    {
        string result;
        Config config;
        config.FromString(service->ConfigLine());
        string version = service->Version();

        if (LocationService::IsSupported(config.Source.Value()) == Core::ERROR_NONE) {
            _skipURL = static_cast<uint16_t>(service->WebPrefix().length());
            _source = config.Source.Value();
            _service = service;

            _sink.Initialize(service, config.Source.Value(), config.Interval.Value(), config.Retries.Value());
        } else {
            result = _T("URL for retrieving location is incorrect !!!");
        }

        // On success return empty, to indicate there is no error text.
        return (result);
    }

    /* virtual */ void LocationSync::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        _sink.Deinitialize();
    }

    /* virtual */ string LocationSync::Information() const
    {
        // No additional info to report.
        return (string());
    }

    /* virtual */ void LocationSync::Inbound(Web::Request& /* request */)
    {
    }

    /* virtual */ Core::ProxyType<Web::Response>
    LocationSync::Process(const Web::Request& request)
    {
        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        Core::TextSegmentIterator index(
            Core::TextFragment(request.Path, _skipURL, static_cast<uint16_t>(request.Path.length()) - _skipURL),
            false,
            '/');

        // By default, we assume everything works..
        result->ErrorCode = Web::STATUS_OK;
        result->Message = "OK";

        if (request.Verb == Web::Request::HTTP_GET) {
            Core::ProxyType<Web::JSONBodyType<Data>> response(jsonResponseFactory.Element());

            PluginHost::ISubSystem* subSystem = _service->SubSystems();

            ASSERT(subSystem != nullptr);

            const PluginHost::ISubSystem::IInternet* internet(subSystem->Get<PluginHost::ISubSystem::IInternet>());
            const PluginHost::ISubSystem::ILocation* location(subSystem->Get<PluginHost::ISubSystem::ILocation>());

            response->PublicIp = internet->PublicIPAddress();
            response->TimeZone = location->TimeZone();
            response->Region = location->Region();
            response->Country = location->Country();
            response->City = location->City();

            result->ContentType = Web::MIMETypes::MIME_JSON;
            result->Body(Core::proxy_cast<Web::IBody>(response));
        } else if (request.Verb == Web::Request::HTTP_POST) {
            index.Next();
            if (index.Next()) {
                if ((index.Current() == "Sync") && (_source.empty() == false)) {
                    uint32_t error = _sink.Probe(_source, 1, 1);

                    if (error != Core::ERROR_NONE) {
                        result->ErrorCode = Web::STATUS_INTERNAL_SERVER_ERROR;
                        result->Message = _T("Probe failed with error code: ") + Core::NumberType<uint32_t>(error).Text();
                    }
                }
            }
        } else {
            result->ErrorCode = Web::STATUS_BAD_REQUEST;
            result->Message = _T("Unsupported request for the [LocationSync] service.");
        }

        return result;
    }

    void LocationSync::SyncedLocation()
    {
        PluginHost::ISubSystem* subSystem = _service->SubSystems();

        ASSERT(subSystem != nullptr);

        if (subSystem != nullptr) {

            subSystem->Set(PluginHost::ISubSystem::INTERNET, _sink.Network());
            subSystem->Set(PluginHost::ISubSystem::LOCATION, _sink.Location());
            subSystem->Release();

            if ((_sink.Location() != nullptr) && (_sink.Location()->TimeZone().empty() == false)) {
                Core::SystemInfo::SetEnvironment(_T("TZ"), _sink.Location()->TimeZone());
                event_locationchange();
            }
        }
    }

    int LocationSync::tzDecode(char *p)
    {
        return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    }

    char *LocationSync::timefmt(char *p, int len, int interval)
    {
    	int		hours=0, mins=0, secs=0;
    	const char		*fmt, *sign;

    	/* XXX Verify for negative values of interval. */
    	sign = "";
    	if ( interval < 0 ) {
    		sign = "-";
    		interval = -interval;
    	}
    
    	secs = interval % 60;
    	interval -= secs;
    	interval /= 60;
    	mins = interval % 60;
    	interval -= mins;
    	hours = interval / 60;
    
    	fmt = ( (secs != 0)
    			? "%s%d:%d:%d"
    			: ( (mins != 0)
    					? "%s%d:%d"
    					: "%s%d" ) );
    
    	snprintf(p, len, fmt, sign, hours, mins, secs);
    
    	return p;
    }

    /*
    ** Determine n'th week holding wday.
    */
    int LocationSync::weekofmonth(int mday)
    {
    	int	tmp;
        
    	tmp = 1 + mday/7;
    	/* Assume that the last week of the month is desired. */
    	if ( tmp == 4 )
    		tmp++;
    
    	return tmp;
    }


    void LocationSync::getZoneInfo(std::string file, std::string &zoneInfo)
    {

        struct stat ziStat;
        if (stat(file.c_str(), &ziStat))
        {
            TRACE_L1("stat() failed: %s", strerror(errno));
            zoneInfo = "";
            return;
        }

        if (ziStat.st_size < sizeof (struct tzhead))
        {
            TRACE_L1("%s is too small for zoneinfo", file.c_str());
            zoneInfo = "";
            return;
        }

        std::vector <char> buf;
        buf.resize(ziStat.st_size);

        size_t r = 0;
        FILE *f = fopen(file.c_str(), "r");
        if (f)
        {
            r = fread(buf.data(), 1, ziStat.st_size, f);
            if (ziStat.st_size != r)
            {
                TRACE_L1("Failed to read zoneinfo");
                fclose(f);
                zoneInfo = "";
                return;
            }
            fclose(f);
        }

        struct tzhead *tzhp = (struct tzhead *)buf.data();

        int ttisstdcnt = tzDecode(tzhp->tzh_ttisstdcnt);
        int leapcnt = tzDecode(tzhp->tzh_leapcnt);
        int timecnt = tzDecode(tzhp->tzh_timecnt);
        int typecnt = tzDecode(tzhp->tzh_typecnt);
        int charcnt = tzDecode(tzhp->tzh_charcnt);

        if ( timecnt > TZ_MAX_TIMES ) {
            TRACE_L1("timecnt too large (%d)\n", timecnt);
            zoneInfo = "";
            return;
        }
        if ( typecnt == 0 ) {
            TRACE_L1("typecnt too small (%d)\n", typecnt);
            zoneInfo = "";
            return;
        }
        if ( typecnt > TZ_MAX_TYPES ) {
            TRACE_L1("typecnt too large (%d)\n", typecnt);
            zoneInfo = "";
            return;
        }
        if ( charcnt > TZ_MAX_CHARS ) {
            TRACE_L1("charcnt too large (%d)\n", charcnt);
            zoneInfo = "";
            return;
        }
        if ( leapcnt > TZ_MAX_LEAPS ) {
            TRACE_L1("leapcnt too large (%d)\n", leapcnt);
            zoneInfo = "";
            return;
        }
        if ( ttisstdcnt > TZ_MAX_TYPES ) {
            TRACE_L1("ttisstdcnt too large (%d)\n", ttisstdcnt);
            zoneInfo = "";
            return;
        }
    #ifdef STRUCT_TZHEAD_TTISGMTCNT
        if ( ttisgmtcnt > TZ_MAX_TYPES ) {
            TRACE_L1("ttisgmtcnt too large (%d)\n", ttisgmtcnt);
            zoneInfo = "";
            return;
        }
    #endif
        if ( ziStat.st_size < sizeof (*tzhp)
                + timecnt * (4 + 1) * sizeof (char)
                + typecnt * (4 + 2) * sizeof (char)
                + charcnt * sizeof (char)
                + leapcnt * (4 + 4) * sizeof (char)
                + ttisstdcnt * sizeof (char)
            )
        {
            TRACE_L1("%s is too small %d\n", file.c_str(), ziStat.st_size);
            zoneInfo = "";
            return;
        }


        struct {
            long	time;
            long	type;
        } transit[TZ_MAX_TIMES];

        struct {
            int	gmtoffset;	/* gmtoffs */
            char	isdst;		/* isdsts */
            char	abbrind;	/* abbrinds, unsigned char?? */
            int	stds;		/* char? */

        } lti[TZ_MAX_TYPES];

        struct {
            time_t	transit;		/* trans */
            long	correct;		/* corr */
            char	roll; 			/* ??? */
        } leaps[TZ_MAX_LEAPS];

        struct {
            time_t		time;
            int		index;
            unsigned char	type;
            struct tm	*tm;
        } tt[2] = {
            { 0, 0, 0, NULL },
            { 0, 0, 0, NULL }
        };

        char		stdoffset[TIMEBUFSIZE], dstoffset[TIMEBUFSIZE];
        char		startdate[TIMEBUFSIZE], enddate[TIMEBUFSIZE];
        char		starttime[TIMEBUFSIZE], endtime[TIMEBUFSIZE];

        char	chars[TZ_MAX_CHARS];
        time_t		now;
        struct tm	tmbuf[2];
        int		startindex, endindex;
        struct tm	*starttm, *endtm;

        char *p;

        /*
        ** Parse the remainder of the zoneinfo data file.
        */

        p = buf.data() + sizeof (*tzhp);

        /* transition times */
        for (int i = 0 ; i < timecnt ; i++)
        {
            transit[i].time = tzDecode(p);
            p += 4;
            /* record the next two (future) transitions */
            if ( transit[i].time > now ) {
                if ( tt[0].time == 0 ) {
                    tt[0].time = transit[i].time;
                    tt[0].index = i;
                }
                else if ( tt[1].time == 0 ) {
                    tt[1].time = transit[i].time;
                    tt[1].index = i;
                }
            }
        }
        /* local time types for above: 0 = std, 1 = dst */
        for (int i = 0 ; i < timecnt ; i++)
        {
            transit[i].type = (unsigned char) *p++;
        }

        /* GMT offset seconds, local time type, abbreviation index */
        for (int i = 0 ; i < typecnt ; i++)
        {
            lti[i].gmtoffset = tzDecode(p);
            p += 4;
            lti[i].isdst = (unsigned char) *p++;
            lti[i].abbrind = (unsigned char) *p++;
        }

        /* timezone abbreviation strings */
        for (int i = 0 ; i < charcnt ; i++)
        {
            chars[i] = *p++;
        }
        chars[charcnt] = '\0';	/* ensure '\0' at end */

        /* leap second transitions, accumulated correction */
        for (int i = 0 ; i < leapcnt ; i++)
        {
            leaps[i].transit = tzDecode(p);
            p += 4;
            leaps[i].correct = tzDecode(p);
            p += 4;
        }

        /*
        ** indexed by type:
        **	0 = transition is wall clock time
        **	1 = transition time is standard time
        **	default (if absent) is wall clock time
        */
        for (int i = 0 ; i < ttisstdcnt ; i++)
        {
            lti[i].stds = *p++;
        }


        /* Simple case of no dst */
        if (typecnt == 1)
        {
            timefmt(stdoffset, sizeof stdoffset, -lti[0].gmtoffset);

            zoneInfo = &chars[lti[0].abbrind];
            zoneInfo += stdoffset;
            return;
        }

        /*
        ** XXX If no transitions exist in the future, should we assume
        ** XXX that dst no longer applies, or should we assume the most
        ** XXX recent rules continue to apply???
        ** XXX For the moment, we assume the latter and proceed.
        */
        if ( tt[0].time == 0 && tt[1].time == 0 ) {
            tt[1].index = timecnt - 1;
            tt[0].index = tt[1].index - 1;
            tt[1].time = transit[tt[1].index].time;
            tt[0].time = transit[tt[0].index].time;
        }
        else if ( tt[1].time == 0 ) {
            tt[1].index = tt[0].index;
            tt[0].index--;
            tt[1].time = transit[tt[1].index].time;
            tt[0].time = transit[tt[0].index].time;
        }

        tt[0].type = transit[tt[0].index].type;
        tt[1].type = transit[tt[1].index].type;


        /*
        ** Convert time_t values to struct tm values.
        */
        for (int i = 0 ; i <= 1 ; i++) {
            time_t	tmptime;

                tmptime = tt[i].time
                        + lti[tt[(i>0)?0:1].type].gmtoffset;
            if ( lti[i].stds != 0 && lti[tt[i].type].isdst != 0 )
                tmptime += lti[tt[i].type].gmtoffset
                        - lti[tt[(i>0)?0:1].type].gmtoffset;
            tt[i].tm = gmtime(&tmptime);
            tt[i].tm = (struct tm*)memcpy(&tmbuf[i], tt[i].tm, sizeof(struct tm));
        }

        if ( lti[tt[0].type].isdst == 1 ) {
            startindex = 0;
            endindex = 1;
        }
        else {
            startindex = 1;
            endindex = 0;
        }
        starttm = tt[startindex].tm;
        endtm = tt[endindex].tm;

        /* XXX This calculation of the week is too simple-minded??? */
        /* XXX A hueristic would be to round 4 up to 5. */
        sprintf(startdate, "M%d.%d.%d",
                1 + starttm->tm_mon,
                weekofmonth(starttm->tm_mday),
                starttm->tm_wday
                );
        sprintf(starttime, "%.2d:%.2d:%.2d",
                starttm->tm_hour,
                starttm->tm_min,
                starttm->tm_sec
                );
        sprintf(enddate, "M%d.%d.%d",
                1 + endtm->tm_mon,
                weekofmonth(endtm->tm_mday),
                endtm->tm_wday
                );
        sprintf(endtime, "%.2d:%.2d:%.2d",
                endtm->tm_hour,
                endtm->tm_min,
                endtm->tm_sec
                );

        timefmt(stdoffset, sizeof stdoffset, -lti[tt[endindex].type].gmtoffset);
        timefmt(dstoffset, sizeof dstoffset, -lti[tt[startindex].type].gmtoffset);

        char strbuf[256];
        snprintf(strbuf, sizeof(strbuf), "%s%s%s%s,%s/%s,%s/%s",
            &chars[lti[tt[endindex].type].abbrind], stdoffset,
            &chars[lti[tt[startindex].type].abbrind], dstoffset,
            startdate, starttime,
            enddate, endtime
            );

        zoneInfo = strbuf;
    }

    void LocationSync::processTimeZones(std::string dir, JsonObject& out)
    {
        DIR *d = opendir(dir.c_str());

        struct dirent *de;

        while ((de = readdir(d)))
        {
            if (0 == de->d_name[0] || 0 == strcmp(de->d_name, ".") || 0 == strcmp(de->d_name, ".."))
                continue;

            std::string fullName = dir;
            fullName += "/";
            fullName += de->d_name;

            struct stat deStat;
            if (stat(fullName.c_str(), &deStat))
            {
                TRACE_L1("stat() failed: %s", strerror(errno));
                continue;
            }

            if (S_ISDIR(deStat.st_mode))
            {
                JsonObject dirObject;
                processTimeZones(fullName, dirObject);
                out[de->d_name] = dirObject;
            }
            else
            {
                if (0 == access(fullName.c_str(), R_OK))
                {
                    std::string zoneInfo;
                    getZoneInfo(fullName, zoneInfo);
                    out[de->d_name] = zoneInfo;
                }
                else
                    TRACE_L1("no access to %s", fullName.c_str());
            }
        }
    }

    uint32_t LocationSync::getTimeZones(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("called");
        TRACE_L1("called");

        JsonObject dirObject;
        processTimeZones(ZONEINFO_DIR, dirObject);

        response["zoneinfo"] = dirObject;

        return Core::ERROR_NONE;
    }

} // namespace Plugin
} // namespace WPEFramework
