/**
 *  Copyright (C) 2023 The de.mrh.evspam Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// C / C++
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <deque>

// External
#include <libmrh/Send/MRH_SendEvent.h>
#include <libmrh/MRH_AppLoop.h>
#include <libmrhevdata.h>
#include <libmrhab/MRH_ABLogger.h>

// Project
#include "./Revision.h"

// Pre-defined
#define MRH_EVENT_SPAM_LOOP_COUNT 30

// Namespace
namespace
{
    // Required responses
    std::vector<MRH_Uint32> v_Response;
}


// Prevent name wrangling for library header functions
#ifdef __cplusplus
extern "C"
{
#endif

    //*************************************************************************************
    // Init
    //*************************************************************************************

    int MRH_Init(const MRH_A_SendContext* p_SendContext, const char* p_LaunchInput, int i_LaunchCommandID)
    {
        MRH::AB::Logger& c_Logger = MRH::AB::Logger::Singleton();

        c_Logger.Log(MRH::AB::Logger::INFO, "Initializing event spam application (Version: " +
                                            std::string(REVISION_STRING) +
                                            ")",
                     "Main.cpp", __LINE__);

        // Create all events to send
        std::deque<MRH_Event*> dq_Event;
        MRH_Uint32 u32_LoopCount = 0;

        do
        {
            MRH_Uint32 u32_ToCreate = MRH_EVENT_LISTEN_AVAIL_U;

            do
            {
                // Create event
                MRH_Event* p_Event = MRH_EVD_CreateEvent(u32_ToCreate, NULL, 0);

                if (p_Event == NULL)
                {
                    c_Logger.Log(MRH::AB::Logger::INFO, "Failed to create send event!",
                                 "Main.cpp", __LINE__);
                    return -1;
                }

                dq_Event.emplace_back(p_Event);

                // Next to create
                switch (u32_ToCreate)
                {
                    case MRH_EVENT_LISTEN_AVAIL_U:
                        v_Response.emplace_back(MRH_EVENT_LISTEN_AVAIL_S);
                        u32_ToCreate = MRH_EVENT_SAY_AVAIL_U;
                        break;
                    case MRH_EVENT_SAY_AVAIL_U:
                        v_Response.emplace_back(MRH_EVENT_SAY_AVAIL_S);
                        u32_ToCreate = MRH_EVENT_PASSWORD_AVAIL_U;
                        break;
                    case MRH_EVENT_PASSWORD_AVAIL_U:
                        v_Response.emplace_back(MRH_EVENT_PASSWORD_AVAIL_S);
                        u32_ToCreate = MRH_EVENT_USER_AVAIL_U;
                        break;
                    case MRH_EVENT_USER_AVAIL_U:
                        v_Response.emplace_back(MRH_EVENT_USER_AVAIL_S);
                        u32_ToCreate = MRH_EVENT_APP_AVAIL_U;
                        break;
                    case MRH_EVENT_APP_AVAIL_U:
                        v_Response.emplace_back(MRH_EVENT_APP_AVAIL_S);
                        u32_ToCreate = MRH_EVENT_UNK;
                        break;

                    default:
                        u32_ToCreate = MRH_EVENT_UNK;
                        break;
                }
            }
            while (u32_ToCreate != MRH_EVENT_UNK);
        }
        while ((u32_LoopCount += 1) < MRH_EVENT_SPAM_LOOP_COUNT);

        // Send all
        while (dq_Event.empty() == false)
        {
            MRH_Event* p_Send = dq_Event.front();
            dq_Event.pop_front();

            switch (MRH_A_SendEvent(p_SendContext, &p_Send)) /* Consumes p_Event */
            {
                case MRH_A_Send_Result::MRH_A_SEND_FAILURE:
                    c_Logger.Log(MRH::AB::Logger::ERROR, "Failed to send output event!",
                                 "Main.cpp", __LINE__);
                    return -1;

                case MRH_A_Send_Result::MRH_A_SEND_OK:
                    c_Logger.Log(MRH::AB::Logger::INFO, "Sent output event.",
                                 "Main.cpp", __LINE__);

                default:
                    break;
            }
        }

        return 0;
    }

    //*************************************************************************************
    // Update
    //*************************************************************************************

    int MRH_Update(const MRH_Event* p_Event)
    {
        for (auto It = v_Response.begin(); It != v_Response.end(); ++It)
        {
            if (*It == p_Event->u32_Type)
            {
                MRH::AB::Logger::Singleton().Log(MRH::AB::Logger::INFO, std::to_string(v_Response.size()) +
                                                                        " remaining responses required.",
                                                 "Main.cpp", __LINE__);
                v_Response.erase(It);
                break;
            }
        }

        if (v_Response.empty() == true)
        {
            return -1;
        }

        MRH::AB::Logger::Singleton().Log(MRH::AB::Logger::INFO, "All responses received, stopping.",
                                             "Main.cpp", __LINE__);
        return 0;
    }

    //*************************************************************************************
    // Exit
    //*************************************************************************************

    void MRH_Exit(void)
    {}

#ifdef __cplusplus
}
#endif
