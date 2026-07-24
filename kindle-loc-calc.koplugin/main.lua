--[[--
Kindle Locations Calculator, by Mario Montalvo, 23/07/2026
--]]--


local Dispatcher = require("dispatcher")  -- luacheck:ignore
local InfoMessage = require("ui/widget/infomessage")
local UIManager = require("ui/uimanager")
local WidgetContainer = require("ui/widget/container/widgetcontainer")
local DataStorage = require("datastorage")
local _ = require("gettext")
local logger = require("logger")


local KLocs = WidgetContainer:extend{
    name = "kindle_locs",
	title = _("Kindle Locations"),
    is_doc_only = true,
}

function KLocs:onDispatcherRegisterActions()
    Dispatcher:registerAction("kindle_loc_action", {category="none", event="KindleLocations", title=_("Kindle Locations"), general=true,})
end

function KLocs:init()
    self:onDispatcherRegisterActions()
    self.ui.menu:registerToMainMenu(self)
	self.pageno = 0
end



function KLocs:getInfoString()
    local file_path = self.ui.document.file
    local path = DataStorage:getFullDataDir()
    local bin_path = path .. "/plugins/kindle-loc-calc.koplugin/epub_html_sumsize/epub_html_sumsize"
    local pages = self.ui.document:getPageCount()
    local pageno = self.pageno

    -- porcentaje de avance
    local percent_finished = pageno / pages

    -- comando del binario
    local bin_cmd = bin_path .. " \"" .. file_path .. "\""
    logger.info("sys call: " .. bin_cmd)

    -- ejecutar binario y capturar salida
    local handle = io.popen(bin_cmd)
    local raw_output = handle:read("*a")
    handle:close()

    local HTML_bytes = tonumber(raw_output)

    if not HTML_bytes then
        return "Error: binary returned non-numeric output.\nRaw output:\n" .. tostring(raw_output)
    end

    if HTML_bytes < 0 then
        -- aquí puedes mapear tus códigos de error
        local errmap = {
			[-1] = "File is not an .epub or no argument was provided.",
			[-2] = "File does not exist in the filesystem.",
			[-3] = "Cannot open file as ZIP (mz_zip_reader_init_file failed).",
			[-4] = "Unexpected generic error.",
			[-5] = "Corrupt EPUB: ZIP opened but central directory cannot be read.",
			[-6] = "Valid EPUB but contains no HTML/HTM/XHTML files."
        }
        local msg = errmap[HTML_bytes] or "Unknown error."
        return "Error code " .. HTML_bytes .. ": " .. msg
    end

    -- cálculo de locations según MobileRead
    local locations = math.floor(HTML_bytes / 150 + 1)
    local location = math.floor(percent_finished * locations)
    local page_to_loc_equivalence = locations / pages


    local info =
        "Total locations in book: " .. locations .. "\n" ..
        "Current location: " .. location .. "\n" ..
        "Total pages in book: " .. pages .. "\n" ..
        "Current page: " .. pageno .. "\n" ..
        string.format("Positions per page: %.2f", page_to_loc_equivalence)

    return info
end


function KLocs:onPageUpdate(pageno)
	self.pageno = pageno
end

function KLocs:addToMainMenu(menu_items)
    menu_items.hello_world = {
        text = _("Kindle Locations"),
        -- in which menu this should be appended
        sorting_hint = "tools",
        -- a callback when tapping
        callback = function()
            UIManager:show(InfoMessage:new{
                text = _(self:getInfoString()),
            })
        end,
    }
end

function KLocs:onKindleLocations()
    local popup = InfoMessage:new{
        text = _(self:getInfoString()),
    }
    UIManager:show(popup)
end

return KLocs
