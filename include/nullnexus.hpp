/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "websocketclient.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// Reference implementation of nullnexus client
class NullNexus
{
    std::unique_ptr<WebSocketClient> ws;
    bool settings = false;
    std::string username;

    // Callbacks
    std::optional<std::function<bool(boost::property_tree::ptree tree)>> callback_custom;
    std::optional<std::function<void(std::string username, std::string message)>> callback_chat;

    bool sendAuthenticatedMessage(bool reliable, std::string type, boost::property_tree::ptree child)
    {
        if (!ws)
            return false;
        boost::property_tree::ptree pt;
        // Basic data
        pt.put("username", username);
        pt.put("type", type);

        // Data exclusive to this request
        pt.put_child("data", child);

        std::ostringstream buf;
        write_json(buf, pt, false);
        return ws->sendMessage(buf.str(), reliable);
    }

    void handleMessage_chat(boost::property_tree::ptree tree)
    {
        auto data = tree.get_child("data");
        if (callback_chat)
            (*callback_chat)(data.get<std::string>("user"), data.get<std::string>("msg"));
    }

    void handleMessage(std::string msg)
    {
        std::cout << "CO: New message: " << msg << std::endl;
        try {
            // Parse message
            std::istringstream iss (msg);
            boost::property_tree::ptree pt;
            boost::property_tree::read_json (iss, pt);

            if (callback_custom)
                // If the custom callback handled this message, we should stop
                if ((*callback_custom)(pt))
                    return;

            std::string type = pt.get<std::string>("type");
            std::cout << type << std::endl;
            if (type == "chat")
                handleMessage_chat(pt);
        }  catch (...) { }
    }

public:
    // Change some setting
    void changeSettings(std::optional<std::string> name = std::nullopt)
    {
        settings = true;
        username = name ? *name : "Anon-" + std::to_string(rand() % 9000 + 1000);
    }
    // Connect to a specific server
    void connect(bool state = true, std::string host = "localhost", std::string port = "3000", std::string endpoint = "/client/v1")
    {
        if (!settings)
            changeSettings();
        if (state)
        {
            ws = std::make_unique<WebSocketClient>(host, port, endpoint, std::bind(&NullNexus::handleMessage, this, std::placeholders::_1));
            ws->start();
        }
        else
            ws->stop();
    }
    // Send a chat message
    bool sendChat(std::string message, std::string location = "public")
    {
        if (message.size() > 128 || message.size() < 1)
            return false;
        if (std::count_if(message.begin(), message.end(), [](unsigned char c) { return !std::isprint(c); }) != 0)
            return false;

        boost::property_tree::ptree pt;
        pt.put("msg", message);
        pt.put("loc", location);
        return sendAuthenticatedMessage(false, "chat", pt);
    }
    // Add a handler that overrides all other handlers implemented by this class.
    // Return true if your handler handled the message, false if you want the class to handle it.
    void setHandlerCustom(std::function<bool(boost::property_tree::ptree tree)> handler)
    {
        callback_custom = handler;
    }
    // Handle chat messages
    void setHandlerChat(std::function<void(std::string username, std::string message)> handler)
    {
        callback_chat = handler;
    }
};
