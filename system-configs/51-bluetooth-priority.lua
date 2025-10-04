rule = {
  matches = {
    {
      { "device.name", "matches", "bluez_card.*" },
    },
  },
  apply_properties = {
    ["priority.driver"] = 1000,
    ["priority.session"] = 1000,
    ["device.profile.name"] = "headset-head-unit",
  },
}

table.insert(alsa_monitor.rules, rule)

bluez_priority = {
  matches = {
    {
      { "node.name", "matches", "bluez_output.*" },
    },
  },
  apply_properties = {
    ["priority.driver"] = 1500,
    ["priority.session"] = 1500,
  },
}

table.insert(alsa_monitor.rules, bluez_priority)
