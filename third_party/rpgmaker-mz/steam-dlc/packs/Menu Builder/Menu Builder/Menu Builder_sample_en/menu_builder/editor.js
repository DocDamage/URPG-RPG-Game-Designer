function WINDOW_TEXT_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        try{
            obj['Text'] = JSON.parse(obj['Text']);
        }catch(e){
            obj['Text'] = "";
        }
        return obj
    }catch(e){
        return;
    }
}

function GRAPHIC_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj
    }catch(e){
        return;
    }
}

function STATIC_GRAPHIC_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj
    }catch(e){
        const obj = {};
        obj['File'] = "";
        obj['X'] = "0";
        obj['Y'] = "0";
        obj['Scrolling X'] = "0";
        obj['Scrolling Y'] = "0";
        obj['Anchor X'] = "0";
        obj['Anchor Y'] = "0";
        obj['Rotation'] = "0";
        obj['Constant Rotation'] = "false";
        return obj;
    }
}

function ANIMATED_GRAPHIC_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj
    }catch(e){
        const obj = {};
        obj['File'] = "";
        obj['X'] = "0";
        obj['Y'] = "0";
        obj['Max Frames'] = "0";
        obj['Frame Rate'] = "0";
        return obj;
    }
}

function DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        obj['X'] = eval(obj['X']);
        obj['Y'] = eval(obj['Y']);
        obj['Width'] = eval(obj['Width']);
        obj['Height'] = eval(obj['Height']);
    }catch(e){
        console.warn(`Failed to parse dimension configuration. ${e}`);
        const obj = {};
        obj['X'] = 0;
        obj['Y'] = 0;
        obj['Width'] = 1;
        obj['Height'] = 1;
    }
    return obj;
}

function WINDOW_STYLE_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj)
        obj['Font Size'] = eval(obj['Font Size']);
        obj['Font Outline Thickness'] = eval(obj['Font Outline Thickness']);
        obj['Window Opacity'] = eval(obj['Window Opacity']);
        obj['Show Window Dimmer'] = eval(obj['Show Window Dimmer']);
    }catch(e){
        console.warn(`Failed to parse window style. ${e}`);
        const obj = {};
        obj['Font Size'] = 16;
        obj['Font Face'] = 'sans-serif';
        obj['Base Font Color'] = '#ffffff';
        obj['Font Outline Color'] = 'rgba(0, 0, 0, 0.5)';
        obj['Font Outline Thickness'] = 3;
        obj['Window Skin'] = 'Window';
        obj['Window Opacity'] = 255;
        obj['Show Window Dimmer'] = false;
    }
    return obj;
}

function WINDOW_DISPLAY_REQUIREMENTS_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        try{
            obj['Code'] = JSON.parse(obj['Code']);
        }catch(e){
            obj['Code'] = "";
        }
        return obj;
    }catch(e){
        const obj = {};
        obj['Game Switch'] = "0";
        obj['Game Variable'] = "0";
        obj['Variable Minimum'] = "0";
        obj['Variable Maximum'] = "0";
        obj['Code'] = "";
        return obj;
    }
}

function GAUGE_DRAW_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj;
    }catch(e){
        return;
    }
}

function BASIC_WINDOW_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
        obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
        obj['Display Requirements'] = WINDOW_DISPLAY_REQUIREMENTS_PARSER_MNUBLD(obj['Display Requirements']);
        try{
            const texts = JSON.parse(obj['Draw Texts']).map((text_config)=>{
                return WINDOW_TEXT_PARSER_MNUBLD(text_config);
            }).filter(Boolean);
            obj['Draw Texts'] = texts;
        }catch(e){
            obj['Draw Texts'] = [];
        }
        try{
            const codes = JSON.parse(obj['Text References'])
            obj['Text References'] = codes;
        }catch(e){
            obj['Text References'] = [];
        }
        try{
            const pictures = JSON.parse(obj['Draw Pictures']).map((pic_config)=>{
                return GRAPHIC_PARSER_MNUBLD(pic_config);
            }).filter(Boolean);
            obj['Draw Pictures'] = pictures;
        }catch(e){
            obj['Draw Pictures'] = [];
        }
        try{
            obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                return GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
            }).filter(Boolean)
        }catch(e){
            obj['Gauges'] = [];
        }
        return obj;
    }catch(e){
        return;
    }
}

function ACTOR_BASE_PARAM_WINDOW_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj;
    }catch(e){
        return;
    }
}

function ACTOR_EX_PARAM_WINDOW_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj;
    }catch(e){
        return;
    }
}

function ACTOR_SP_PARAM_WINDOW_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj;
    }catch(e){
        return;
    }
}

function ACTOR_DATA_WINDOW_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
        obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
        obj['Display Requirements'] = WINDOW_DISPLAY_REQUIREMENTS_PARSER_MNUBLD(obj['Display Requirements']);
        try{
            obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                return GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
            }).filter(Boolean)
        }catch(e){
            obj['Gauges'] = [];
        }
        try{
            obj['Draw Base Params'] = JSON.parse(obj['Draw Base Params']).map((data)=>{
                return ACTOR_BASE_PARAM_WINDOW_PARSER_MNUBLD(data);
            }).filter(Boolean);
        }catch(e){
            obj['Draw Base Params'] = [];
        }
        try{
            obj['Draw Ex Params'] = JSON.parse(obj['Draw Ex Params']).map((data)=>{
                return ACTOR_EX_PARAM_WINDOW_PARSER_MNUBLD(data);
            }).filter(Boolean);
        }catch(e){
            obj['Draw Ex Params'] = [];
        }
        try{
            obj['Draw Sp Params'] = JSON.parse(obj['Draw Sp Params']).map((data)=>{
                return ACTOR_SP_PARAM_WINDOW_PARSER_MNUBLD(data);
            }).filter(Boolean);
        }catch(e){
            obj['Draw Sp Params'] = [];
        }
        return obj;
    }catch(e){
        return;
    }
}

function ACTOR_SELECT_WINDOW_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
        obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
        try{
            obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                return GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
            }).filter(Boolean)
        }catch(e){
            obj['Gauges'] = [];
        }
        try{
            obj['Draw Base Params'] = JSON.parse(obj['Draw Base Params']).map((data)=>{
                return ACTOR_BASE_PARAM_WINDOW_PARSER_MNUBLD(data);
            }).filter(Boolean);
        }catch(e){
            obj['Draw Base Params'] = [];
        }
        try{
            obj['Draw Ex Params'] = JSON.parse(obj['Draw Ex Params']).map((data)=>{
                return ACTOR_EX_PARAM_WINDOW_PARSER_MNUBLD(data);
            }).filter(Boolean);
        }catch(e){
            obj['Draw Ex Params'] = [];
        }
        try{
            obj['Draw Sp Params'] = JSON.parse(obj['Draw Sp Params']).map((data)=>{
                return ACTOR_SP_PARAM_WINDOW_PARSER_MNUBLD(data);
            }).filter(Boolean);
        }catch(e){
            obj['Draw Sp Params'] = [];
        }
        return obj;
    }catch(e){
        return;
    }
}

function VAR_REQ_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj;
    }catch(e){
        console.error(e);
        return;
    }
}

function ITM_REQ_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj;
    }catch(e){
        return;
    }
}

function WEP_REQ_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj;
    }catch(e){
        return;
    }
}

function ARM_REQ_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj;
    }catch(e){
        return;
    }
}

function SELC_OPT_REQ_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        try{
            obj['Variable Requirements'] = JSON.parse(obj['Variable Requirements']).map((config)=>{
                return VAR_REQ_PARSER_MNUBLD(config);
            }).filter(Boolean)
        }catch(e){
            obj['Variable Requirements'] = [];
        }
        try{
            obj['Switch Requirements'] = JSON.parse(obj['Switch Requirements']);
        }catch(e){
            obj['Switch Requirements'] = [];
        }
        try{
            obj['Item Requirements'] = JSON.parse(obj['Item Requirements']).map((config)=>{
                return ITM_REQ_PARSER_MNUBLD(config);
            }).filter(Boolean)
        }catch(e){
            obj['Item Requirements'] = [];
        }
        try{
            obj['Weapon Requirements'] = JSON.parse(obj['Weapon Requirements']).map((config)=>{
                return WEP_REQ_PARSER_MNUBLD(config);
            }).filter(Boolean)
        }catch(e){
            obj['Weapon Requirements'] = [];
        }
        try{
            obj['Armor Requirements'] = JSON.parse(obj['Armor Requirements']).map((config)=>{
                return ARM_REQ_PARSER_MNUBLD(config);
            }).filter(Boolean)
        }catch(e){
            obj['Armor Requirements'] = [];
        }
        return obj;
    }catch(e){
        const obj = {};
        obj['Variable Requirements'] = [];
        obj['Switch Requirements'] = [];
        obj['Item Requirements'] = [];
        obj['Weapon Requirements'] = [];
        obj['Armor Requirements'] = [];
        return obj;
    }
}

function SELC_OPT_BUTTON_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        obj['Cold Graphic'] = ANIMATED_GRAPHIC_PARSER_MNUBLD(obj['Cold Graphic']);
        obj['Hot Graphic'] = ANIMATED_GRAPHIC_PARSER_MNUBLD(obj['Hot Graphic']);
        return obj;
    }catch(e){
        const obj = {};
        obj['Name'] = "";
        obj['X'] = "0";
        obj['Y'] = "0";
        obj['Cold Graphic'] = ANIMATED_GRAPHIC_PARSER_MNUBLD(obj['Cold Graphic']);
        obj['Hot Graphic'] = ANIMATED_GRAPHIC_PARSER_MNUBLD(obj['Hot Graphic']);
        return obj;
    }
}

function SELC_OPT_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        obj['Display Requirements'] = SELC_OPT_REQ_PARSER_MNUBLD(obj['Display Requirements']);
        obj['Select Requirements'] = SELC_OPT_REQ_PARSER_MNUBLD(obj['Select Requirements']);
        obj['Static Graphic'] = STATIC_GRAPHIC_PARSER_MNUBLD(obj['Static Graphic']);
        obj['Animated Graphic'] = ANIMATED_GRAPHIC_PARSER_MNUBLD(obj['Animated Graphic']);
        obj['Scene Button'] = SELC_OPT_BUTTON_PARSER_MNUBLD(obj['Scene Button']);
        try{
            obj['Pictures'] = JSON.parse(obj['Pictures']);
        }catch(e){
            obj['Pictures'] = [];
        }
        try{
            obj['Description'] = JSON.parse(obj['Description']);
        }catch(e){
            obj['Description'] = "";
        }
        try{
            obj['Code Execution'] = JSON.parse(obj['Code Execution']);
        }catch(e){
            obj['Code Execution'] = "";
        }
        return obj;
    }catch(e){
        return;
    }
}

function SELC_WINDOW_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
        obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
        try{
            obj['Selection Options'] = JSON.parse(obj['Selection Options']).map((opt_config)=>{
                return SELC_OPT_PARSER_MNUBLD(opt_config);
            }).filter(Boolean)
        }catch(e){
            obj['Selection Options'] = [];
        }
        try{
            obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                return GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
            }).filter(Boolean)
        }catch(e){
            obj['Gauges'] = [];
        }
        return obj;
    }catch(e){
        return console.error(e);
    }
}

function SELC_DATA_WINDOW_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
        obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
        obj['Display Requirements'] = WINDOW_DISPLAY_REQUIREMENTS_PARSER_MNUBLD(obj['Display Requirements']);
        try{
            obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                return GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
            }).filter(Boolean)
        }catch(e){
            obj['Gauges'] = [];
        }
        return obj;
    }catch(e){
        return;
    }
}

function MENU_SCENE_BUILD_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        if(!obj['Identifier Name']){
            console.error(`No identifier name detected. Parse aborted.`);
            return;
        }
        try{
            obj['Preload Backgrounds'] = JSON.parse(obj['Preload Backgrounds']);
        }catch(e){
            obj['Preload Backgrounds'] = [];
        }
        try{
            obj['On Load Script Calls'] = JSON.parse(obj['On Load Script Calls']).map((script_json)=>{
                try{
                    script_json = JSON.parse(script_json);
                    return script_json;
                }catch(e){
                    return ''
                }
            }).filter(Boolean);
        }catch(e){
            obj['On Load Script Calls'] = [];
        }
        try{
            obj['Backgrounds'] = JSON.parse(obj['Backgrounds']).map((bg_config)=>{
                return STATIC_GRAPHIC_PARSER_MNUBLD(bg_config);
            }).filter(Boolean);
        }catch(e){
            obj['Backgrounds'] = [];
        }
        try{
            obj['Back Graphics'] = JSON.parse(obj['Back Graphics']).map((bg_config)=>{
                return ANIMATED_GRAPHIC_PARSER_MNUBLD(bg_config);
            }).filter(Boolean);
        }catch(e){
            obj['Back Graphics'] = [];
        }
        try{
            obj['Update Codes'] = JSON.parse(obj['Update Codes']).map((code_json)=>{
                try{
                    code_json = JSON.parse(code_json);
                    return code_json;
                }catch(e){
                    return "";
                }
            }).filter(Boolean)
        }catch(e){
            obj['Update Codes'] = [];
        }
        obj['Selection Window'] = SELC_WINDOW_PARSER_MNUBLD(obj['Selection Window']);
        obj['Actor Selection Window'] = ACTOR_SELECT_WINDOW_PARSER_MNUBLD(obj['Actor Selection Window']);
        try{
            obj['Actor Data Windows'] = JSON.parse(obj['Actor Data Windows']).map((config)=>{
                return ACTOR_DATA_WINDOW_PARSER_MNUBLD(config);
            }).filter(Boolean);
        }catch(e){
            obj['Actor Data Windows'] = [];
        }
        try{
            obj['Selection Data Windows'] = JSON.parse(obj['Selection Data Windows']).map((config)=>{
                return SELC_DATA_WINDOW_PARSER_MNUBLD(config);
            }).filter(Boolean);
        }catch(e){
            obj['Selection Data Windows'] = [];
        }
        try{
            const basic_windows = JSON.parse(obj['Basic Windows']).map((window_config)=>{
                return BASIC_WINDOW_PARSER_MNUBLD(window_config);
            }).filter(Boolean)
            obj['Basic Windows'] = basic_windows;
        }catch(e){
            obj['Basic Windows'] = [];
        }
        try{
            obj['Foregrounds'] = JSON.parse(obj['Foregrounds']).map((fg_config)=>{
                return STATIC_GRAPHIC_PARSER_MNUBLD(fg_config);
            }).filter(Boolean);
        }catch(e){
            obj['Foregrounds'] = [];
        }
        try{
            obj['Fore Graphics'] = JSON.parse(obj['Fore Graphics']).map((fg_config)=>{
                return ANIMATED_GRAPHIC_PARSER_MNUBLD(fg_config);
            }).filter(Boolean);
        }catch(e){
            obj['Fore Graphics'] = [];
        }
        return obj;
    }catch(e){
        return;
    }
}

function SCENE_OVERRIDE_PARSER_MNUBLD(obj){
    try{
        obj = JSON.parse(obj);
        return obj;
    }catch(e){
        return;
    }
}

const clearDocument = function(){
    const body = document.getElementById('body');
    body.innerHTML = "";
}

const createTabButtons = function(){
    const tab_div = document.getElementById("Main_Menu");
    const scene_button = document.createElement("button");
    scene_button.id = 'scene_menu_button';
    scene_button.textContent = 'MENU SETUP';
    scene_button.addEventListener(`click`, ()=>{
        createMenuListContents()
    })
    tab_div.appendChild(scene_button);
    const override_button = document.createElement("button");
    override_button.id = 'override_menu_button';
    override_button.textContent = 'OVERRIDE SETUP';
    override_button.addEventListener(`click`, ()=>{
        createOverrideListContents()
    })
    tab_div.appendChild(override_button);
    const reset_button = document.createElement("button");
    reset_button.id = 'reset_menu_button';
    reset_button.textContent = 'RESET';
    reset_button.addEventListener(`click`, ()=>{
        load_editor();
    })
    tab_div.appendChild(reset_button);
}

const createTabs = function(){
    const body = document.getElementById('body');
    const tab_div = document.createElement('div');
    tab_div.id = "Main_Menu";
    body.appendChild(tab_div);
    createTabButtons();
}

const createContents = function(){
    const body = document.getElementById('body');
    const contents = document.createElement("div");
    contents.id = 'contents';
    body.appendChild(contents);
}

const clearContents = function(){
    const contents = document.getElementById('contents');
    contents.innerHTML = "";
}

const createMenuIdNameForm = function(form, data){
    const id_name_label = document.createElement('label');
    id_name_label.id = "identifier_name_label";
    id_name_label.textContent = "Identifier Name: ";
    form.appendChild(id_name_label);
    const id_name = document.createElement('input');
    id_name.id = "identifier_name_input";
    id_name.type = "text";
    id_name.setAttribute("value", data['Identifier Name']);
    id_name.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        recompileMenus(menu_data, menu);
    })
    form.appendChild(id_name);
}

const createOnloadScriptsForm = function(form, data){
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "On Load Scripts";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "On_Load_Scripts_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const script_container = document.createElement('div');
    script_container.id = "On_Load_Scripts_Container";
    script_container.classList.add("content");
    const existing_scripts = data['On Load Script Calls'] || [];
    existing_scripts.forEach((script_note)=>{
        const script_div = document.createElement('div');
        script_div.id = "script_line_container";
        script_container.appendChild(script_div);
        const label = document.createElement("label");
        label.id = "Script_Label";
        label.textContent = "Script";
        const input = document.createElement('input');
        input.id = "Script_Input";
        input.setAttribute("size", 50);
        input.setAttribute("value", script_note);
        input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = menu['On Load Script Calls'].indexOf(script_note);
            if(sav_index >= 0){
                menu['On Load Script Calls'][sav_index] = input.value;
                recompileMenus(menu_data, menu);
            }
        })
        script_div.appendChild(label);
        script_div.appendChild(input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const del_index = menu['On Load Script Calls'].indexOf(script_note);
            if(del_index >= 0){
                menu['On Load Script Calls'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        script_div.appendChild(delete_button);
    })
    const add_script_button = document.createElement('button');
    add_script_button.id = "Add_Button";
    add_script_button.type = 'button';
    add_script_button.textContent = "ADD SCRIPT";
    add_script_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        menu['On Load Script Calls'].push("console.log(`Added script. Edit to remove.`)");
        recompileMenus(menu_data, menu, true);
    })
    script_container.appendChild(add_script_button);
    form.appendChild(script_container);
}

const createBackgroundsForm = function(form, data){
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Backgrounds";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const background_container = document.createElement('div');
    background_container.id = "Button_Container";
    background_container.classList.add("content");
    const existing_backgrounds = data['Backgrounds'] || [];
    existing_backgrounds.forEach((background_data)=>{
        const file_name = background_data['File'];
        const background_div = document.createElement('div');
        background_div.id = "background_data_container";
        background_container.appendChild(background_div);
        const bg_file_div = document.createElement('div');
        bg_file_div.id = 'Background_Element_Div'
        const file_label = document.createElement("label");
        file_label.id = "BG_Label";
        file_label.textContent = `File: ${file_name}`;
        background_div.appendChild(file_label);
        const input_file = document.createElement('input');
        input_file.id = "BG_Input";
        input_file.setAttribute("type", "file");
        input_file.setAttribute("accept", "image/png");
        input_file.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_file.value){
                    const input_paths = (input_file.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
                    if(input_paths.length > 0){
                        const path = input_paths[0];
                        let file_name = path.replace("img\\pictures\\", "");
                        file_name = file_name.replace(".png", "");
                        file_name = file_name.replace("\\", "/");
                        menu['Backgrounds'][sav_index]['File'] = file_name;
                        file_label.textContent = `File: ${file_name}`;
                        background_data['File'] = file_name;
                    }
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_file);
        const X_label = document.createElement("label");
        X_label.id = "BG_Label";
        X_label.textContent = `X: ${background_data['X']}`;
        background_div.appendChild(X_label);
        const input_X = document.createElement('input');
        input_X.id = "BG_Input";
        input_X.setAttribute("size", 15);
        input_X.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_X.value){
                    let value = input_X.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Backgrounds'][sav_index]['X'] = value;
                    X_label.textContent = `X: ${value}`;
                    background_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_X);
        const Y_label = document.createElement("label");
        Y_label.id = "BG_Label";
        Y_label.textContent = `Y: ${background_data['Y']}`;
        background_div.appendChild(Y_label);
        const input_Y = document.createElement('input');
        input_Y.id = "BG_Input";
        input_Y.setAttribute("size", 15);
        input_Y.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_Y.value){
                    let value = input_Y.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Backgrounds'][sav_index]['Y'] = value;
                    Y_label.textContent = `Y: ${value}`;
                    background_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_Y);
        const SX_label = document.createElement("label");
        SX_label.id = "BG_Label";
        SX_label.textContent = `Scrolling X: ${background_data['Scrolling X']}`;
        background_div.appendChild(SX_label);
        const input_SX = document.createElement('input');
        input_SX.id = "BG_Input";
        input_SX.setAttribute("size", 15);
        input_SX.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_SX.value){
                    let value = input_SX.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Backgrounds'][sav_index]['Scrolling X'] = value;
                    SX_label.textContent = `Scrolling X: ${value}`;
                    background_data['Scrolling X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_SX);
        const SY_label = document.createElement("label");
        SY_label.id = "BG_Label";
        SY_label.textContent = `Scrolling Y: ${background_data['Scrolling Y']}`;
        background_div.appendChild(SY_label);
        const input_SY = document.createElement('input');
        input_SY.id = "BG_Input";
        input_SY.setAttribute("size", 15);
        input_SY.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_SY.value){
                    let value = input_SY.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Backgrounds'][sav_index]['Scrolling Y'] = value;
                    SY_label.textContent = `Scrolling Y: ${value}`;
                    background_data['Scrolling Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_SY);
        const AX_label = document.createElement("label");
        AX_label.id = "BG_Label";
        AX_label.textContent = `Anchor X: ${background_data['Anchor X']}`;
        background_div.appendChild(AX_label);
        const input_AX = document.createElement('input');
        input_AX.id = "BG_Input";
        input_AX.setAttribute("size", 15);
        input_AX.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_AX.value){
                    let value = input_AX.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Backgrounds'][sav_index]['Anchor X'] = value;
                    AX_label.textContent = `Anchor X: ${value}`;
                    background_data['Anchor X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_AX);
        const AY_label = document.createElement("label");
        AY_label.id = "BG_Label";
        AY_label.textContent = `Anchor Y: ${background_data['Anchor Y']}`;
        background_div.appendChild(AY_label);
        const input_AY = document.createElement('input');
        input_AY.id = "BG_Input";
        input_AY.setAttribute("size", 15);
        input_AY.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_AY.value){
                    let value = input_AY.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Backgrounds'][sav_index]['Anchor Y'] = value;
                    AY_label.textContent = `Anchor Y: ${value}`;
                    background_data['Anchor Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_AY);
        const rot_label = document.createElement('label');
        rot_label.id = "BG_Label";
        rot_label.textContent = `Rotation: ${background_data['Rotation']} (${eval(background_data['Constant Rotation']) ? "constant" : "static"})`;
        background_div.appendChild(rot_label);
        const rot_value = document.createElement('input');
        rot_value.id = "BG_Input";
        rot_value.setAttribute("size", 15);
        rot_value.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(rot_value.value){
                    let value = rot_value.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Backgrounds'][sav_index]['Rotation'] = value;
                    rot_label.textContent = `Rotation: ${value} (${eval(background_data['Constant Rotation']) ? 'constant' : 'static'})`;
                    background_data['Rotation'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(rot_value);
        const rot_constant = document.createElement("input");
        rot_constant.id = 'BG_Check';
        rot_constant.setAttribute("type", "checkbox");
        rot_constant.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                const value = rot_constant.checked;
                menu['Backgrounds'][sav_index]['Constant Rotation'] = value;
                rot_label.textContent = `Rotation: ${background_data['Rotation']} (${value ? 'constant' : 'static'})`;
                background_data['Constant Rotation'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        background_div.appendChild(rot_constant);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let del_index = -1;
            for(let i = 0; i < menu['Backgrounds'].length; i++){
                const bg_menu = menu['Backgrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Backgrounds'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        background_div.appendChild(delete_button);
    })
    const add_background_button = document.createElement('button');
    add_background_button.id = "Add_Button";
    add_background_button.type = 'button';
    add_background_button.textContent = "ADD BACKGROUND";
    add_background_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        menu['Backgrounds'].push({
            "File":"",
            "X":"0",
            "Y":"0",
            "Scrolling X":"0",
            "Scrolling Y":"0",
            "Anchor X":"0",
            "Anchor Y":"0",
            "Rotation":"0",
            "Constant Rotation":"false"
        });
        recompileMenus(menu_data, menu, true);
    })
    background_container.appendChild(add_background_button);
    form.appendChild(background_container);
}

const createForegroundsForm = function(form, data){
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Foregrounds";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const background_container = document.createElement('div');
    background_container.id = "Button_Container";
    background_container.classList.add("content");
    const existing_backgrounds = data['Foregrounds'] || [];
    existing_backgrounds.forEach((background_data)=>{
        const file_name = background_data['File'];
        const background_div = document.createElement('div');
        background_div.id = "background_data_container";
        background_container.appendChild(background_div);
        const bg_file_div = document.createElement('div');
        bg_file_div.id = 'Background_Element_Div'
        const file_label = document.createElement("label");
        file_label.id = "BG_Label";
        file_label.textContent = `File: ${file_name}`;
        background_div.appendChild(file_label);
        const input_file = document.createElement('input');
        input_file.id = "BG_Input";
        input_file.setAttribute("type", "file");
        input_file.setAttribute("accept", "image/png");
        input_file.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_file.value){
                    const input_paths = (input_file.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
                    if(input_paths.length > 0){
                        const path = input_paths[0];
                        let file_name = path.replace("img\\pictures\\", "");
                        file_name = file_name.replace(".png", "");
                        file_name = file_name.replace("\\", "/");
                        menu['Foregrounds'][sav_index]['File'] = file_name;
                        file_label.textContent = `File: ${file_name}`;
                        background_data['File'] = file_name;
                    }
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_file);
        const X_label = document.createElement("label");
        X_label.id = "BG_Label";
        X_label.textContent = `X: ${background_data['X']}`;
        background_div.appendChild(X_label);
        const input_X = document.createElement('input');
        input_X.id = "BG_Input";
        input_X.setAttribute("size", 15);
        input_X.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_X.value){
                    let value = input_X.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Foregrounds'][sav_index]['X'] = value;
                    X_label.textContent = `X: ${value}`;
                    background_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_X);
        const Y_label = document.createElement("label");
        Y_label.id = "BG_Label";
        Y_label.textContent = `Y: ${background_data['Y']}`;
        background_div.appendChild(Y_label);
        const input_Y = document.createElement('input');
        input_Y.id = "BG_Input";
        input_Y.setAttribute("size", 15);
        input_Y.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_Y.value){
                    let value = input_Y.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Foregrounds'][sav_index]['Y'] = value;
                    Y_label.textContent = `Y: ${value}`;
                    background_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_Y);
        const SX_label = document.createElement("label");
        SX_label.id = "BG_Label";
        SX_label.textContent = `Scrolling X: ${background_data['Scrolling X']}`;
        background_div.appendChild(SX_label);
        const input_SX = document.createElement('input');
        input_SX.id = "BG_Input";
        input_SX.setAttribute("size", 15);
        input_SX.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_SX.value){
                    let value = input_SX.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Foregrounds'][sav_index]['Scrolling X'] = value;
                    SX_label.textContent = `Scrolling X: ${value}`;
                    background_data['Scrolling X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_SX);
        const SY_label = document.createElement("label");
        SY_label.id = "BG_Label";
        SY_label.textContent = `Scrolling Y: ${background_data['Scrolling Y']}`;
        background_div.appendChild(SY_label);
        const input_SY = document.createElement('input');
        input_SY.id = "BG_Input";
        input_SY.setAttribute("size", 15);
        input_SY.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_SY.value){
                    let value = input_SY.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Foregrounds'][sav_index]['Scrolling Y'] = value;
                    SY_label.textContent = `Scrolling Y: ${value}`;
                    background_data['Scrolling Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_SY);
        const AX_label = document.createElement("label");
        AX_label.id = "BG_Label";
        AX_label.textContent = `Anchor X: ${background_data['Anchor X']}`;
        background_div.appendChild(AX_label);
        const input_AX = document.createElement('input');
        input_AX.id = "BG_Input";
        input_AX.setAttribute("size", 15);
        input_AX.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_AX.value){
                    let value = input_AX.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Foregrounds'][sav_index]['Anchor X'] = value;
                    AX_label.textContent = `Anchor X: ${value}`;
                    background_data['Anchor X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_AX);
        const AY_label = document.createElement("label");
        AY_label.id = "BG_Label";
        AY_label.textContent = `Anchor Y: ${background_data['Anchor Y']}`;
        background_div.appendChild(AY_label);
        const input_AY = document.createElement('input');
        input_AY.id = "BG_Input";
        input_AY.setAttribute("size", 15);
        input_AY.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_AY.value){
                    let value = input_AY.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Foregrounds'][sav_index]['Anchor Y'] = value;
                    AY_label.textContent = `Anchor Y: ${value}`;
                    background_data['Anchor Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(input_AY);
        const rot_label = document.createElement('label');
        rot_label.id = "BG_Label";
        rot_label.textContent = `Rotation: ${background_data['Rotation']} (${eval(background_data['Constant Rotation']) ? "constant" : "static"})`;
        background_div.appendChild(rot_label);
        const rot_value = document.createElement('input');
        rot_value.id = "BG_Input";
        rot_value.setAttribute("size", 15);
        rot_value.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(rot_value.value){
                    let value = rot_value.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Backgrounds'][sav_index]['Rotation'] = value;
                    rot_label.textContent = `Rotation: ${value} (${eval(background_data['Constant Rotation']) ? 'constant' : 'static'})`;
                    background_data['Rotation'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        background_div.appendChild(rot_value);
        const rot_constant = document.createElement("input");
        rot_constant.id = 'BG_Check';
        rot_constant.setAttribute("type", "checkbox");
        rot_constant.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                const value = rot_constant.checked;
                menu['Foregrounds'][sav_index]['Constant Rotation'] = value;
                rot_label.textContent = `Rotation: ${background_data['Rotation']} (${value ? 'constant' : 'static'})`;
                background_data['Constant Rotation'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        background_div.appendChild(rot_constant);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let del_index = -1;
            for(let i = 0; i < menu['Foregrounds'].length; i++){
                const bg_menu = menu['Foregrounds'][i];
                if(bg_menu){
                    if(bg_menu['File'] == background_data['File']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Foregrounds'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        background_div.appendChild(delete_button);
    })
    const add_background_button = document.createElement('button');
    add_background_button.id = "Add_Button";
    add_background_button.type = 'button';
    add_background_button.textContent = "ADD FOREGROUND";
    add_background_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        menu['Foregrounds'].push({
            "File":"",
            "X":"0",
            "Y":"0",
            "Scrolling X":"0",
            "Scrolling Y":"0",
            "Anchor X":"0",
            "Anchor Y":"0",
            "Rotation":"0",
            "Constant Rotation":"false"
        });
        recompileMenus(menu_data, menu, true);
    })
    background_container.appendChild(add_background_button);
    form.appendChild(background_container);
}

const createBackGfxForm = function(form, data){
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Back Graphics";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const backgfx_container = document.createElement('div');
    backgfx_container.id = "Button_Container";
    backgfx_container.classList.add("content");
    const existing_back_gfx = data['Back Graphics'] || [];
    existing_back_gfx.forEach((back_gfx_data)=>{
        const file_name = back_gfx_data['File'];
        const backgfx_div = document.createElement('div');
        backgfx_div.id = "background_data_container";
        backgfx_container.appendChild(backgfx_div);
        const bg_file_div = document.createElement('div');
        bg_file_div.id = 'Background_Element_Div'
        const file_label = document.createElement("label");
        file_label.id = "BG_Label";
        file_label.textContent = `File: ${file_name}`;
        backgfx_div.appendChild(file_label);
        const input_file = document.createElement('input');
        input_file.id = "BG_Input";
        input_file.setAttribute("size", 25);
        input_file.setAttribute("type", "file");
        input_file.setAttribute("accept", "image/png");
        input_file.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Back Graphics'].length; i++){
                const bg_menu = menu['Back Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_file.value){
                    const input_paths = (input_file.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
                    if(input_paths.length > 0){
                        const path = input_paths[0];
                        let file_name = path.replace("img\\pictures\\", "");
                        file_name = file_name.replace(".png", "");
                        file_name = file_name.replace("\\", "/");
                        menu['Back Graphics'][sav_index]['File'] = file_name;
                        file_label.textContent = `File: ${file_name}`;
                        back_gfx_data['File'] = file_name;
                    }
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_file);
        const X_label = document.createElement("label");
        X_label.id = "BG_Label";
        X_label.textContent = `X: ${back_gfx_data['X']}`;
        backgfx_div.appendChild(X_label);
        const input_X = document.createElement('input');
        input_X.id = "BG_Input";
        input_X.setAttribute("size", 15);
        input_X.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Back Graphics'].length; i++){
                const bg_menu = menu['Back Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_X.value){
                    let value = input_X.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Back Graphics'][sav_index]['X'] = value;
                    X_label.textContent = `X: ${value}`;
                    back_gfx_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_X);
        const Y_label = document.createElement("label");
        Y_label.id = "BG_Label";
        Y_label.textContent = `Y: ${back_gfx_data['Y']}`;
        backgfx_div.appendChild(Y_label);
        const input_Y = document.createElement('input');
        input_Y.id = "BG_Input";
        input_Y.setAttribute("size", 15);
        input_Y.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Back Graphics'].length; i++){
                const bg_menu = menu['Back Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_Y.value){
                    let value = input_Y.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Back Graphics'][sav_index]['Y'] = value;
                    Y_label.textContent = `Y: ${value}`;
                    back_gfx_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_Y);
        const max_frames_label = document.createElement("label");
        max_frames_label.id = "BG_Label";
        max_frames_label.textContent = `Max Frames: ${back_gfx_data['Max Frames']}`;
        backgfx_div.appendChild(max_frames_label);
        const input_max_frames = document.createElement('input');
        input_max_frames.id = "BG_Input";
        input_max_frames.setAttribute("size", 15);
        input_max_frames.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Back Graphics'].length; i++){
                const bg_menu = menu['Back Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_max_frames.value){
                    let value = input_max_frames.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Back Graphics'][sav_index]['Max Frames'] = value;
                    max_frames_label.textContent = `Max Frames: ${value}`;
                    back_gfx_data['Max Frames'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_max_frames);
        const frame_rate_label = document.createElement("label");
        frame_rate_label.id = "BG_Label";
        frame_rate_label.textContent = `Frame Rate: ${back_gfx_data['Frame Rate']}`;
        backgfx_div.appendChild(frame_rate_label);
        const input_frame_rate = document.createElement('input');
        input_frame_rate.id = "BG_Input";
        input_frame_rate.setAttribute("size", 15);
        input_frame_rate.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Back Graphics'].length; i++){
                const bg_menu = menu['Back Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_frame_rate.value){
                    let value = input_frame_rate.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Back Graphics'][sav_index]['Frame Rate'] = value;
                    frame_rate_label.textContent = `Frame Rate: ${value}`;
                    back_gfx_data['Frame Rate'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_frame_rate);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let del_index = -1;
            for(let i = 0; i < menu['Back Graphics'].length; i++){
                const bg_menu = menu['Back Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Back Graphics'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        backgfx_div.appendChild(delete_button);
    })
    const add_backgfx_button = document.createElement('button');
    add_backgfx_button.id = "Add_Button";
    add_backgfx_button.type = 'button';
    add_backgfx_button.textContent = "ADD BACK GRAPHIC";
    add_backgfx_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        menu['Back Graphics'].push({
            "File":"",
            "X":"0",
            "Y":"0",
            "Max Frames":"0",
            "Frame Rate":"0",
        });
        recompileMenus(menu_data, menu, true);
    })
    backgfx_container.appendChild(add_backgfx_button);
    form.appendChild(backgfx_container);
}

const createForeGfxForm = function(form, data){
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Fore Graphics";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const backgfx_container = document.createElement('div');
    backgfx_container.id = "Button_Container";
    backgfx_container.classList.add("content");
    const existing_back_gfx = data['Fore Graphics'] || [];
    existing_back_gfx.forEach((back_gfx_data)=>{
        const file_name = back_gfx_data['File'];
        const backgfx_div = document.createElement('div');
        backgfx_div.id = "background_data_container";
        backgfx_container.appendChild(backgfx_div);
        const bg_file_div = document.createElement('div');
        bg_file_div.id = 'Background_Element_Div'
        const file_label = document.createElement("label");
        file_label.id = "BG_Label";
        file_label.textContent = `File: ${file_name}`;
        backgfx_div.appendChild(file_label);
        const input_file = document.createElement('input');
        input_file.id = "BG_Input";
        input_file.setAttribute("size", 25);
        input_file.setAttribute("type", "file");
        input_file.setAttribute("accept", "image/png");
        input_file.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Fore Graphics'].length; i++){
                const bg_menu = menu['Fore Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_file.value){
                    const input_paths = (input_file.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
                    if(input_paths.length > 0){
                        const path = input_paths[0];
                        let file_name = path.replace("img\\pictures\\", "");
                        file_name = file_name.replace(".png", "");
                        file_name = file_name.replace("\\", "/");
                        menu['Fore Graphics'][sav_index]['File'] = file_name;
                        file_label.textContent = `File: ${file_name}`;
                        back_gfx_data['File'] = file_name;
                    }
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_file);
        const X_label = document.createElement("label");
        X_label.id = "BG_Label";
        X_label.textContent = `X: ${back_gfx_data['X']}`;
        backgfx_div.appendChild(X_label);
        const input_X = document.createElement('input');
        input_X.id = "BG_Input";
        input_X.setAttribute("size", 15);
        input_X.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Fore Graphics'].length; i++){
                const bg_menu = menu['Fore Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_X.value){
                    let value = input_X.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Fore Graphics'][sav_index]['X'] = value;
                    X_label.textContent = `X: ${value}`;
                    back_gfx_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_X);
        const Y_label = document.createElement("label");
        Y_label.id = "BG_Label";
        Y_label.textContent = `Y: ${back_gfx_data['Y']}`;
        backgfx_div.appendChild(Y_label);
        const input_Y = document.createElement('input');
        input_Y.id = "BG_Input";
        input_Y.setAttribute("size", 15);
        input_Y.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Fore Graphics'].length; i++){
                const bg_menu = menu['Fore Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_Y.value){
                    let value = input_Y.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Fore Graphics'][sav_index]['Y'] = value;
                    Y_label.textContent = `Y: ${value}`;
                    back_gfx_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_Y);
        const max_frames_label = document.createElement("label");
        max_frames_label.id = "BG_Label";
        max_frames_label.textContent = `Max Frames: ${back_gfx_data['Max Frames']}`;
        backgfx_div.appendChild(max_frames_label);
        const input_max_frames = document.createElement('input');
        input_max_frames.id = "BG_Input";
        input_max_frames.setAttribute("size", 15);
        input_max_frames.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Fore Graphics'].length; i++){
                const bg_menu = menu['Fore Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_max_frames.value){
                    let value = input_max_frames.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Fore Graphics'][sav_index]['Max Frames'] = value;
                    max_frames_label.textContent = `Max Frames: ${value}`;
                    back_gfx_data['Max Frames'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_max_frames);
        const frame_rate_label = document.createElement("label");
        frame_rate_label.id = "BG_Label";
        frame_rate_label.textContent = `Frame Rate: ${back_gfx_data['Frame Rate']}`;
        backgfx_div.appendChild(frame_rate_label);
        const input_frame_rate = document.createElement('input');
        input_frame_rate.id = "BG_Input";
        input_frame_rate.setAttribute("size", 15);
        input_frame_rate.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            for(let i = 0; i < menu['Fore Graphics'].length; i++){
                const bg_menu = menu['Fore Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(input_frame_rate.value){
                    let value = input_frame_rate.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Fore Graphics'][sav_index]['Frame Rate'] = value;
                    frame_rate_label.textContent = `Frame Rate: ${value}`;
                    back_gfx_data['Frame Rate'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        backgfx_div.appendChild(input_frame_rate);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let del_index = -1;
            for(let i = 0; i < menu['Fore Graphics'].length; i++){
                const bg_menu = menu['Fore Graphics'][i];
                if(bg_menu){
                    if(bg_menu['File'] == back_gfx_data['File']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Fore Graphics'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        backgfx_div.appendChild(delete_button);
    })
    const add_backgfx_button = document.createElement('button');
    add_backgfx_button.id = "Add_Button";
    add_backgfx_button.type = 'button';
    add_backgfx_button.textContent = "ADD FORE GRAPHIC";
    add_backgfx_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        menu['Fore Graphics'].push({
            "File":"",
            "X":"0",
            "Y":"0",
            "Max Frames":"0",
            "Frame Rate":"0",
        });
        recompileMenus(menu_data, menu, true);
    })
    backgfx_container.appendChild(add_backgfx_button);
    form.appendChild(backgfx_container);
}

const addWindowDimensionForm = function(container, data){
    const selc_window = data['Selection Window'];
    const init_x_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['X'] : 0;
    const init_y_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Y'] : 0;
    const init_w_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Width'] : 1;
    const init_h_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Height'] : 1;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Dimension Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const dimension_config_container = document.createElement('div');
    dimension_config_container.id = "Button_Container";
    dimension_config_container.classList.add("content");
    container.appendChild(dimension_config_container);
    const x_label = document.createElement('label');
    x_label.id = "Window_Label";
    x_label.textContent = `X: ${init_x_value}`;
    dimension_config_container.appendChild(x_label);
    const x_input = document.createElement('input');
    x_input.id = "Window_Input";
    x_input.setAttribute("size", 15);
    x_input.setAttribute("value", init_x_value);
    x_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = x_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Window']['Dimension Configuration']['X'] = value;
            x_label.textContent = `X: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(x_input);
    const y_label = document.createElement('label');
    y_label.id = "Window_Label";
    y_label.textContent = `Y: ${init_y_value}`;
    dimension_config_container.appendChild(y_label);
    const y_input = document.createElement('input');
    y_input.id = "Window_Input";
    y_input.setAttribute("size", 15);
    y_input.setAttribute("value", init_y_value);
    y_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = y_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Window']['Dimension Configuration']['Y'] = value;
            y_label.textContent = `Y: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(y_input);
    const w_label = document.createElement('label');
    w_label.id = "Window_Label";
    w_label.textContent = `Width: ${init_w_value}`;
    dimension_config_container.appendChild(w_label);
    const w_input = document.createElement('input');
    w_input.id = "Window_Input";
    w_input.setAttribute("size", 15);
    w_input.setAttribute("value", init_w_value);
    w_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = w_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Window']['Dimension Configuration']['Width'] = value;
            w_label.textContent = `Width: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(w_input);
    const h_label = document.createElement('label');
    h_label.id = "Window_Label";
    h_label.textContent = `Height: ${init_h_value}`;
    dimension_config_container.appendChild(h_label);
    const h_input = document.createElement('input');
    h_input.id = "Window_Input";
    h_input.setAttribute("size", 15);
    h_input.setAttribute("value", init_h_value);
    h_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = h_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Window']['Dimension Configuration']['Height'] = value;
            h_label.textContent = `Height: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(h_input);
}

const addWindowDataDimensionForm = function(container, data, index){
    const selc_windows = data['Selection Data Windows'];
    const selc_window = selc_windows[index];
    const init_x_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['X'] : 0;
    const init_y_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Y'] : 0;
    const init_w_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Width'] : 1;
    const init_h_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Height'] : 1;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Dimension Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const dimension_config_container = document.createElement('div');
    dimension_config_container.id = "Button_Container";
    dimension_config_container.classList.add("content");
    container.appendChild(dimension_config_container);
    const x_label = document.createElement('label');
    x_label.id = "Window_Label";
    x_label.textContent = `X: ${init_x_value}`;
    dimension_config_container.appendChild(x_label);
    const x_input = document.createElement('input');
    x_input.id = "Window_Input";
    x_input.setAttribute("size", 15);
    x_input.setAttribute("value", init_x_value);
    x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = x_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Dimension Configuration']['X'] = value;
            x_label.textContent = `X: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(x_input);
    const y_label = document.createElement('label');
    y_label.id = "Window_Label";
    y_label.textContent = `Y: ${init_y_value}`;
    dimension_config_container.appendChild(y_label);
    const y_input = document.createElement('input');
    y_input.id = "Window_Input";
    y_input.setAttribute("size", 15);
    y_input.setAttribute("value", init_y_value);
    y_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = y_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Dimension Configuration']['Y'] = value;
            y_label.textContent = `Y: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(y_input);
    const w_label = document.createElement('label');
    w_label.id = "Window_Label";
    w_label.textContent = `Width: ${init_w_value}`;
    dimension_config_container.appendChild(w_label);
    const w_input = document.createElement('input');
    w_input.id = "Window_Input";
    w_input.setAttribute("size", 15);
    w_input.setAttribute("value", init_w_value);
    w_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = w_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Dimension Configuration']['Width'] = value;
            w_label.textContent = `Width: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(w_input);
    const h_label = document.createElement('label');
    h_label.id = "Window_Label";
    h_label.textContent = `Height: ${init_h_value}`;
    dimension_config_container.appendChild(h_label);
    const h_input = document.createElement('input');
    h_input.id = "Window_Input";
    h_input.setAttribute("size", 15);
    h_input.setAttribute("value", init_h_value);
    h_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = h_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Dimension Configuration']['Height'] = value;
            h_label.textContent = `Height: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(h_input);
}

const addActrSelcWindowDimensionForm = function(container, data){
    const selc_window = data['Actor Selection Window'];
    const init_x_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['X'] : 0;
    const init_y_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Y'] : 0;
    const init_w_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Width'] : 1;
    const init_h_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Height'] : 1;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Dimension Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const dimension_config_container = document.createElement('div');
    dimension_config_container.id = "Button_Container";
    dimension_config_container.classList.add("content");
    container.appendChild(dimension_config_container);
    const x_label = document.createElement('label');
    x_label.id = "Window_Label";
    x_label.textContent = `X: ${init_x_value}`;
    dimension_config_container.appendChild(x_label);
    const x_input = document.createElement('input');
    x_input.id = "Window_Input";
    x_input.setAttribute("size", 15);
    x_input.setAttribute("value", init_x_value);
    x_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = x_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Selection Window']['Dimension Configuration']['X'] = value;
            x_label.textContent = `X: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(x_input);
    const y_label = document.createElement('label');
    y_label.id = "Window_Label";
    y_label.textContent = `Y: ${init_y_value}`;
    dimension_config_container.appendChild(y_label);
    const y_input = document.createElement('input');
    y_input.id = "Window_Input";
    y_input.setAttribute("size", 15);
    y_input.setAttribute("value", init_y_value);
    y_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = y_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Selection Window']['Dimension Configuration']['Y'] = value;
            y_label.textContent = `Y: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(y_input);
    const w_label = document.createElement('label');
    w_label.id = "Window_Label";
    w_label.textContent = `Width: ${init_w_value}`;
    dimension_config_container.appendChild(w_label);
    const w_input = document.createElement('input');
    w_input.id = "Window_Input";
    w_input.setAttribute("size", 15);
    w_input.setAttribute("value", init_w_value);
    w_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = w_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Selection Window']['Dimension Configuration']['Width'] = value;
            w_label.textContent = `Width: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(w_input);
    const h_label = document.createElement('label');
    h_label.id = "Window_Label";
    h_label.textContent = `Height: ${init_h_value}`;
    dimension_config_container.appendChild(h_label);
    const h_input = document.createElement('input');
    h_input.id = "Window_Input";
    h_input.setAttribute("size", 15);
    h_input.setAttribute("value", init_h_value);
    h_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = h_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Selection Window']['Dimension Configuration']['Height'] = value;
            h_label.textContent = `Height: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(h_input);
}

const addActrDataWindowDimensionForm = function(container, data, index){
    const selc_windows = data['Actor Data Windows'];
    const selc_window = selc_windows[index];
    const init_x_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['X'] : 0;
    const init_y_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Y'] : 0;
    const init_w_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Width'] : 1;
    const init_h_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Height'] : 1;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Dimension Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const dimension_config_container = document.createElement('div');
    dimension_config_container.id = "Button_Container";
    dimension_config_container.classList.add("content");
    container.appendChild(dimension_config_container);
    const x_label = document.createElement('label');
    x_label.id = "Window_Label";
    x_label.textContent = `X: ${init_x_value}`;
    dimension_config_container.appendChild(x_label);
    const x_input = document.createElement('input');
    x_input.id = "Window_Input";
    x_input.setAttribute("size", 15);
    x_input.setAttribute("value", init_x_value);
    x_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = x_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Dimension Configuration']['X'] = value;
            x_label.textContent = `X: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(x_input);
    const y_label = document.createElement('label');
    y_label.id = "Window_Label";
    y_label.textContent = `Y: ${init_y_value}`;
    dimension_config_container.appendChild(y_label);
    const y_input = document.createElement('input');
    y_input.id = "Window_Input";
    y_input.setAttribute("size", 15);
    y_input.setAttribute("value", init_y_value);
    y_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = y_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Dimension Configuration']['Y'] = value;
            y_label.textContent = `Y: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(y_input);
    const w_label = document.createElement('label');
    w_label.id = "Window_Label";
    w_label.textContent = `Width: ${init_w_value}`;
    dimension_config_container.appendChild(w_label);
    const w_input = document.createElement('input');
    w_input.id = "Window_Input";
    w_input.setAttribute("size", 15);
    w_input.setAttribute("value", init_w_value);
    w_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = w_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Dimension Configuration']['Width'] = value;
            w_label.textContent = `Width: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(w_input);
    const h_label = document.createElement('label');
    h_label.id = "Window_Label";
    h_label.textContent = `Height: ${init_h_value}`;
    dimension_config_container.appendChild(h_label);
    const h_input = document.createElement('input');
    h_input.id = "Window_Input";
    h_input.setAttribute("size", 15);
    h_input.setAttribute("value", init_h_value);
    h_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = h_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Dimension Configuration']['Height'] = value;
            h_label.textContent = `Height: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(h_input);
}

const addWindowBasicDimensionForm = function(container, data, index){
    const selc_windows = data['Basic Windows'];
    const selc_window = selc_windows[index];
    const init_x_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['X'] : 0;
    const init_y_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Y'] : 0;
    const init_w_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Width'] : 1;
    const init_h_value = selc_window['Dimension Configuration'] ? selc_window['Dimension Configuration']['Height'] : 1;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Dimension Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const dimension_config_container = document.createElement('div');
    dimension_config_container.id = "Button_Container";
    dimension_config_container.classList.add("content");
    container.appendChild(dimension_config_container);
    const x_label = document.createElement('label');
    x_label.id = "Window_Label";
    x_label.textContent = `X: ${init_x_value}`;
    dimension_config_container.appendChild(x_label);
    const x_input = document.createElement('input');
    x_input.id = "Window_Input";
    x_input.setAttribute("size", 15);
    x_input.setAttribute("value", init_x_value);
    x_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = x_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Dimension Configuration']['X'] = value;
            x_label.textContent = `X: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(x_input);
    const y_label = document.createElement('label');
    y_label.id = "Window_Label";
    y_label.textContent = `Y: ${init_y_value}`;
    dimension_config_container.appendChild(y_label);
    const y_input = document.createElement('input');
    y_input.id = "Window_Input";
    y_input.setAttribute("size", 15);
    y_input.setAttribute("value", init_y_value);
    y_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = y_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Dimension Configuration']['Y'] = value;
            y_label.textContent = `Y: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(y_input);
    const w_label = document.createElement('label');
    w_label.id = "Window_Label";
    w_label.textContent = `Width: ${init_w_value}`;
    dimension_config_container.appendChild(w_label);
    const w_input = document.createElement('input');
    w_input.id = "Window_Input";
    w_input.setAttribute("size", 15);
    w_input.setAttribute("value", init_w_value);
    w_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = w_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Dimension Configuration']['Width'] = value;
            w_label.textContent = `Width: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(w_input);
    const h_label = document.createElement('label');
    h_label.id = "Window_Label";
    h_label.textContent = `Height: ${init_h_value}`;
    dimension_config_container.appendChild(h_label);
    const h_input = document.createElement('input');
    h_input.id = "Window_Input";
    h_input.setAttribute("size", 15);
    h_input.setAttribute("value", init_h_value);
    h_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = h_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Dimension Configuration']['Height'] = value;
            h_label.textContent = `Height: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    dimension_config_container.appendChild(h_input);
}

const addWindowStyleForm = function(container, data){
    const selc_window = data['Selection Window'];
    const has_style = !!selc_window['Window Font and Style Configuration'];
    const init_font_size_value = has_style ? selc_window['Window Font and Style Configuration']['Font Size'] : 16;
    const init_font_face_value = has_style ? selc_window['Window Font and Style Configuration']['Font Face'] : 'sans-serif';
    const init_font_color_value = has_style ? selc_window['Window Font and Style Configuration']['Base Font Color'] : '#ffffff';
    const init_font_outline_color_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Color'] : 'rgba(0, 0, 0, 0.5)';
    const init_font_outline_thickness_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Thickness'] : 3;
    const init_window_skin_value = has_style ? selc_window['Window Font and Style Configuration']['Window Skin'] : "Window";
    const init_window_opacity_value = has_style ? selc_window['Window Font and Style Configuration']['Window Opacity'] : 255;
    const init_window_dimmer_value = has_style ? selc_window['Window Font and Style Configuration']['Show Window Dimmer'] : false;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Window Font and Style Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const style_config_container = document.createElement('div');
    style_config_container.id = "Button_Container";
    style_config_container.classList.add("content");
    container.appendChild(style_config_container);
    const font_size_label = document.createElement('label');
    font_size_label.id = "Window_Label";
    font_size_label.textContent = `Font Size: ${init_font_size_value}`;
    style_config_container.appendChild(font_size_label);
    const font_size_input = document.createElement('input');
    font_size_input.id = "Window_Input";
    font_size_input.setAttribute("size", 15);
    font_size_input.setAttribute("value", init_font_size_value);
    font_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Window']['Window Font and Style Configuration']['Font Size'] = value;
            font_size_label.textContent = `Font Size: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_size_input);
    const font_face_label = document.createElement('label');
    font_face_label.id = "Window_Label";
    font_face_label.textContent = `Font Face: ${init_font_face_value}`;
    style_config_container.appendChild(font_face_label);
    const font_face_input = document.createElement('input');
    font_face_input.id = "Window_Input";
    font_face_input.setAttribute("size", 15);
    font_face_input.setAttribute("value", init_font_face_value);
    font_face_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_face_input.value;
            menu['Selection Window']['Window Font and Style Configuration']['Font Face'] = value;
            font_face_label.textContent = `Font Face: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_face_input);
    const font_color_label = document.createElement('label');
    font_color_label.id = "Window_Label";
    font_color_label.textContent = `Base Font Color: ${init_font_color_value}`;
    style_config_container.appendChild(font_color_label);
    const font_color_input = document.createElement('input');
    font_color_input.id = "Window_Input";
    font_color_input.setAttribute("size", 15);
    font_color_input.setAttribute("value", init_font_color_value);
    font_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_color_input.value;
            if(value[0] != "#"){
                if(value.length > 0){
                    alert("You need to use color hex value.");
                }
            }else{
                menu['Selection Window']['Window Font and Style Configuration']['Base Font Color'] = value;
                font_color_label.textContent = `Base Font Color: ${value}`;
                recompileMenus(menu_data, menu);
            }
        }
    })
    style_config_container.appendChild(font_color_input);
    const font_outline_color_label = document.createElement('label');
    font_outline_color_label.id = "Window_Label";
    font_outline_color_label.textContent = `Font Outline Color: ${init_font_outline_color_value}`;
    style_config_container.appendChild(font_outline_color_label);
    const font_outline_color_input = document.createElement('input');
    font_outline_color_input.id = "Window_Input";
    font_outline_color_input.setAttribute("size", 15);
    font_outline_color_input.setAttribute("value", init_font_outline_color_value);
    font_outline_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_color_input.value;
            menu['Selection Window']['Window Font and Style Configuration']['Font Outline Color'] = value;
            font_outline_color_label.textContent = `Font Outline Color: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_color_input);
    const font_outline_size_label = document.createElement('label');
    font_outline_size_label.id = "Window_Label";
    font_outline_size_label.textContent = `Font Outline Thickness: ${init_font_outline_thickness_value}`;
    style_config_container.appendChild(font_outline_size_label);
    const font_outline_size_input = document.createElement('input');
    font_outline_size_input.id = "Window_Input";
    font_outline_size_input.setAttribute("size", 15);
    font_outline_size_input.setAttribute("value", init_font_outline_thickness_value);
    font_outline_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Window']['Window Font and Style Configuration']['Font Outline Thickness'] = value;
            font_outline_size_label.textContent = `Font Outline Thickness: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_size_input);
    const window_skin_label = document.createElement('label');
    window_skin_label.id = "Window_Label";
    window_skin_label.textContent = `Window Skin: ${init_window_skin_value}`;
    style_config_container.appendChild(window_skin_label);
    const window_skin_input = document.createElement('input');
    window_skin_input.id = "Window_Input";
    window_skin_input.setAttribute("size", 15);
    window_skin_input.setAttribute("type", "file");
    window_skin_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(window_skin_input.value){
            const input_paths = (window_skin_input.value || "").match(/(img\\system\\[a-z\d*|\W\D*]+)/gm);
            if(input_paths.length > 0){
                const path = input_paths[0];
                let file_name = path.replace("img\\system\\", "");
                file_name = file_name.replace(".png", "");
                file_name = file_name.replace("\\", "/");
                menu['Selection Window']['Window Font and Style Configuration']['Window Skin'] = file_name;
                window_skin_label.textContent = `Window Skin: ${file_name}`;
            }
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_skin_input);
    const window_opacity_label = document.createElement('label');
    window_opacity_label.id = "Window_Label";
    window_opacity_label.textContent = `Window Opacity: ${init_window_opacity_value}`;
    style_config_container.appendChild(window_opacity_label);
    const window_opacity_input = document.createElement('input');
    window_opacity_input.id = "Window_Input";
    window_opacity_input.setAttribute("size", 15);
    window_opacity_input.setAttribute("value", init_window_opacity_value);
    window_opacity_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_opacity_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Window']['Window Font and Style Configuration']['Window Opacity'] = value;
            window_opacity_label.textContent = `Window Opacity: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_opacity_input);
    const window_dimmer_label = document.createElement('label');
    window_dimmer_label.id = "Window_Label";
    window_dimmer_label.textContent = `Show Window Dimmer: `;
    style_config_container.appendChild(window_dimmer_label);
    const window_dimmer_input = document.createElement("input");
    window_dimmer_input.id = "Window_Check";
    window_dimmer_input.setAttribute("size", 15);
    window_dimmer_input.setAttribute("type", "checkbox");
    if(init_window_dimmer_value)window_dimmer_input.setAttribute("checked", true);
    window_dimmer_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_dimmer_input.checked;
            menu['Selection Window']['Window Font and Style Configuration']['Show Window Dimmer'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_dimmer_input);
}

const addWindowDataStyleForm = function(container, data, index){
    const selc_windows = data['Selection Data Windows'];
    const selc_window = selc_windows[index];
    const has_style = !!selc_window['Window Font and Style Configuration'];
    const init_font_size_value = has_style ? selc_window['Window Font and Style Configuration']['Font Size'] : 16;
    const init_font_face_value = has_style ? selc_window['Window Font and Style Configuration']['Font Face'] : 'sans-serif';
    const init_font_color_value = has_style ? selc_window['Window Font and Style Configuration']['Base Font Color'] : '#ffffff';
    const init_font_outline_color_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Color'] : 'rgba(0, 0, 0, 0.5)';
    const init_font_outline_thickness_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Thickness'] : 3;
    const init_window_skin_value = has_style ? selc_window['Window Font and Style Configuration']['Window Skin'] : "Window";
    const init_window_opacity_value = has_style ? selc_window['Window Font and Style Configuration']['Window Opacity'] : 255;
    const init_window_dimmer_value = has_style ? selc_window['Window Font and Style Configuration']['Show Window Dimmer'] : false;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Window Font and Style Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const style_config_container = document.createElement('div');
    style_config_container.id = "Button_Container";
    style_config_container.classList.add("content");
    container.appendChild(style_config_container);
    const font_size_label = document.createElement('label');
    font_size_label.id = "Window_Label";
    font_size_label.textContent = `Font Size: ${init_font_size_value}`;
    style_config_container.appendChild(font_size_label);
    const font_size_input = document.createElement('input');
    font_size_input.id = "Window_Input";
    font_size_input.setAttribute("size", 15);
    font_size_input.setAttribute("value", init_font_size_value);
    font_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Window Font and Style Configuration']['Font Size'] = value;
            font_size_label.textContent = `Font Size: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_size_input);
    const font_face_label = document.createElement('label');
    font_face_label.id = "Window_Label";
    font_face_label.textContent = `Font Face: ${init_font_face_value}`;
    style_config_container.appendChild(font_face_label);
    const font_face_input = document.createElement('input');
    font_face_input.id = "Window_Input";
    font_face_input.setAttribute("size", 15);
    font_face_input.setAttribute("value", init_font_face_value);
    font_face_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_face_input.value;
            menu['Selection Data Windows'][index]['Window Font and Style Configuration']['Font Face'] = value;
            font_face_label.textContent = `Font Face: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_face_input);
    const font_color_label = document.createElement('label');
    font_color_label.id = "Window_Label";
    font_color_label.textContent = `Base Font Color: ${init_font_color_value}`;
    style_config_container.appendChild(font_color_label);
    const font_color_input = document.createElement('input');
    font_color_input.id = "Window_Input";
    font_color_input.setAttribute("size", 15);
    font_color_input.setAttribute("value", init_font_color_value);
    font_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_color_input.value;
            if(value[0] != "#"){
                if(value.length > 0){
                    alert("You need to use color hex value.");
                }
            }else{
                menu['Selection Data Windows'][index]['Window Font and Style Configuration']['Base Font Color'] = value;
                font_color_label.textContent = `Base Font Color: ${value}`;
                recompileMenus(menu_data, menu);
            }
        }
    })
    style_config_container.appendChild(font_color_input);
    const font_outline_color_label = document.createElement('label');
    font_outline_color_label.id = "Window_Label";
    font_outline_color_label.textContent = `Font Outline Color: ${init_font_outline_color_value}`;
    style_config_container.appendChild(font_outline_color_label);
    const font_outline_color_input = document.createElement('input');
    font_outline_color_input.id = "Window_Input";
    font_outline_color_input.setAttribute("size", 15);
    font_outline_color_input.setAttribute("value", init_font_outline_color_value);
    font_outline_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_color_input.value;
            menu['Selection Data Windows'][index]['Window Font and Style Configuration']['Font Outline Color'] = value;
            font_outline_color_label.textContent = `Font Outline Color: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_color_input);
    const font_outline_size_label = document.createElement('label');
    font_outline_size_label.id = "Window_Label";
    font_outline_size_label.textContent = `Font Outline Thickness: ${init_font_outline_thickness_value}`;
    style_config_container.appendChild(font_outline_size_label);
    const font_outline_size_input = document.createElement('input');
    font_outline_size_input.id = "Window_Input";
    font_outline_size_input.setAttribute("size", 15);
    font_outline_size_input.setAttribute("value", init_font_outline_thickness_value);
    font_outline_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Window Font and Style Configuration']['Font Outline Thickness'] = value;
            font_outline_size_label.textContent = `Font Outline Thickness: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_size_input);
    const window_skin_label = document.createElement('label');
    window_skin_label.id = "Window_Label";
    window_skin_label.textContent = `Window Skin: ${init_window_skin_value}`;
    style_config_container.appendChild(window_skin_label);
    const window_skin_input = document.createElement('input');
    window_skin_input.id = "Window_Input";
    window_skin_input.setAttribute("size", 15);
    window_skin_input.setAttribute("type", "file");
    window_skin_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(window_skin_input.value){
            const input_paths = (window_skin_input.value || "").match(/(img\\system\\[a-z\d*|\W\D*]+)/gm);
            if(input_paths.length > 0){
                const path = input_paths[0];
                let file_name = path.replace("img\\system\\", "");
                file_name = file_name.replace(".png", "");
                file_name = file_name.replace("\\", "/");
                menu['Selection Data Windows'][index]['Window Font and Style Configuration']['Window Skin'] = file_name;
                window_skin_label.textContent = `Window Skin: ${file_name}`;
            }
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_skin_input);
    const window_opacity_label = document.createElement('label');
    window_opacity_label.id = "Window_Label";
    window_opacity_label.textContent = `Window Opacity: ${init_window_opacity_value}`;
    style_config_container.appendChild(window_opacity_label);
    const window_opacity_input = document.createElement('input');
    window_opacity_input.id = "Window_Input";
    window_opacity_input.setAttribute("size", 15);
    window_opacity_input.setAttribute("value", init_window_opacity_value);
    window_opacity_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_opacity_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Window Font and Style Configuration']['Window Opacity'] = value;
            window_opacity_label.textContent = `Window Opacity: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_opacity_input);
    const window_dimmer_label = document.createElement('label');
    window_dimmer_label.id = "Window_Label";
    window_dimmer_label.textContent = `Show Window Dimmer: `;
    style_config_container.appendChild(window_dimmer_label);
    const window_dimmer_input = document.createElement("input");
    window_dimmer_input.id = "Window_Check";
    window_dimmer_input.setAttribute("size", 15);
    window_dimmer_input.setAttribute("type", "checkbox");
    if(init_window_dimmer_value)window_dimmer_input.setAttribute("checked", true);
    window_dimmer_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_dimmer_input.checked;
            menu['Selection Data Windows'][index]['Window Font and Style Configuration']['Show Window Dimmer'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_dimmer_input);
}

const addActrSelcWindowStyleForm = function(container, data){
    const selc_window = data['Actor Selection Window'];
    const has_style = !!selc_window['Window Font and Style Configuration'];
    const init_font_size_value = has_style ? selc_window['Window Font and Style Configuration']['Font Size'] : 16;
    const init_font_face_value = has_style ? selc_window['Window Font and Style Configuration']['Font Face'] : 'sans-serif';
    const init_font_color_value = has_style ? selc_window['Window Font and Style Configuration']['Base Font Color'] : '#ffffff';
    const init_font_outline_color_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Color'] : 'rgba(0, 0, 0, 0.5)';
    const init_font_outline_thickness_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Thickness'] : 3;
    const init_window_skin_value = has_style ? selc_window['Window Font and Style Configuration']['Window Skin'] : "Window";
    const init_window_opacity_value = has_style ? selc_window['Window Font and Style Configuration']['Window Opacity'] : 255;
    const init_window_dimmer_value = has_style ? selc_window['Window Font and Style Configuration']['Show Window Dimmer'] : false;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Window Font and Style Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const style_config_container = document.createElement('div');
    style_config_container.id = "Button_Container";
    style_config_container.classList.add("content");
    container.appendChild(style_config_container);
    const font_size_label = document.createElement('label');
    font_size_label.id = "Window_Label";
    font_size_label.textContent = `Font Size: ${init_font_size_value}`;
    style_config_container.appendChild(font_size_label);
    const font_size_input = document.createElement('input');
    font_size_input.id = "Window_Input";
    font_size_input.setAttribute("size", 15);
    font_size_input.setAttribute("value", init_font_size_value);
    font_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Selection Window']['Window Font and Style Configuration']['Font Size'] = value;
            font_size_label.textContent = `Font Size: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_size_input);
    const font_face_label = document.createElement('label');
    font_face_label.id = "Window_Label";
    font_face_label.textContent = `Font Face: ${init_font_face_value}`;
    style_config_container.appendChild(font_face_label);
    const font_face_input = document.createElement('input');
    font_face_input.id = "Window_Input";
    font_face_input.setAttribute("size", 15);
    font_face_input.setAttribute("value", init_font_face_value);
    font_face_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_face_input.value;
            menu['Actor Selection Window']['Window Font and Style Configuration']['Font Face'] = value;
            font_face_label.textContent = `Font Face: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_face_input);
    const font_color_label = document.createElement('label');
    font_color_label.id = "Window_Label";
    font_color_label.textContent = `Base Font Color: ${init_font_color_value}`;
    style_config_container.appendChild(font_color_label);
    const font_color_input = document.createElement('input');
    font_color_input.id = "Window_Input";
    font_color_input.setAttribute("size", 15);
    font_color_input.setAttribute("value", init_font_color_value);
    font_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_color_input.value;
            if(value[0] != "#"){
                if(value.length > 0){
                    alert("You need to use color hex value.");
                }
            }else{
                menu['Actor Selection Window']['Window Font and Style Configuration']['Base Font Color'] = value;
                font_color_label.textContent = `Base Font Color: ${value}`;
                recompileMenus(menu_data, menu);
            }
        }
    })
    style_config_container.appendChild(font_color_input);
    const font_outline_color_label = document.createElement('label');
    font_outline_color_label.id = "Window_Label";
    font_outline_color_label.textContent = `Font Outline Color: ${init_font_outline_color_value}`;
    style_config_container.appendChild(font_outline_color_label);
    const font_outline_color_input = document.createElement('input');
    font_outline_color_input.id = "Window_Input";
    font_outline_color_input.setAttribute("size", 15);
    font_outline_color_input.setAttribute("value", init_font_outline_color_value);
    font_outline_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_color_input.value;
            menu['Actor Selection Window']['Window Font and Style Configuration']['Font Outline Color'] = value;
            font_outline_color_label.textContent = `Font Outline Color: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_color_input);
    const font_outline_size_label = document.createElement('label');
    font_outline_size_label.id = "Window_Label";
    font_outline_size_label.textContent = `Font Outline Thickness: ${init_font_outline_thickness_value}`;
    style_config_container.appendChild(font_outline_size_label);
    const font_outline_size_input = document.createElement('input');
    font_outline_size_input.id = "Window_Input";
    font_outline_size_input.setAttribute("size", 15);
    font_outline_size_input.setAttribute("value", init_font_outline_thickness_value);
    font_outline_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Selection Window']['Window Font and Style Configuration']['Font Outline Thickness'] = value;
            font_outline_size_label.textContent = `Font Outline Thickness: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_size_input);
    const window_skin_label = document.createElement('label');
    window_skin_label.id = "Window_Label";
    window_skin_label.textContent = `Window Skin: ${init_window_skin_value}`;
    style_config_container.appendChild(window_skin_label);
    const window_skin_input = document.createElement('input');
    window_skin_input.id = "Window_Input";
    window_skin_input.setAttribute("size", 15);
    window_skin_input.setAttribute("type", "file");
    window_skin_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(window_skin_input.value){
            const input_paths = (window_skin_input.value || "").match(/(img\\system\\[a-z\d*|\W\D*]+)/gm);
            if(input_paths.length > 0){
                const path = input_paths[0];
                let file_name = path.replace("img\\system\\", "");
                file_name = file_name.replace(".png", "");
                file_name = file_name.replace("\\", "/");
                menu['Actor Selection Window']['Window Font and Style Configuration']['Window Skin'] = file_name;
                window_skin_label.textContent = `Window Skin: ${file_name}`;
            }
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_skin_input);
    const window_opacity_label = document.createElement('label');
    window_opacity_label.id = "Window_Label";
    window_opacity_label.textContent = `Window Opacity: ${init_window_opacity_value}`;
    style_config_container.appendChild(window_opacity_label);
    const window_opacity_input = document.createElement('input');
    window_opacity_input.id = "Window_Input";
    window_opacity_input.setAttribute("size", 15);
    window_opacity_input.setAttribute("value", init_window_opacity_value);
    window_opacity_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_opacity_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Selection Window']['Window Font and Style Configuration']['Window Opacity'] = value;
            window_opacity_label.textContent = `Window Opacity: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_opacity_input);
    const window_dimmer_label = document.createElement('label');
    window_dimmer_label.id = "Window_Label";
    window_dimmer_label.textContent = `Show Window Dimmer: `;
    style_config_container.appendChild(window_dimmer_label);
    const window_dimmer_input = document.createElement("input");
    window_dimmer_input.id = "Window_Check";
    window_dimmer_input.setAttribute("size", 15);
    window_dimmer_input.setAttribute("type", "checkbox");
    if(init_window_dimmer_value)window_dimmer_input.setAttribute("checked", true);
    window_dimmer_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_dimmer_input.checked;
            menu['Actor Selection Window']['Window Font and Style Configuration']['Show Window Dimmer'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_dimmer_input);
}

const addActrDataWindowStyleForm = function(container, data, index){
    const selc_windows = data['Actor Data Windows'];
    const selc_window = selc_windows[index];
    const has_style = !!selc_window['Window Font and Style Configuration'];
    const init_font_size_value = has_style ? selc_window['Window Font and Style Configuration']['Font Size'] : 16;
    const init_font_face_value = has_style ? selc_window['Window Font and Style Configuration']['Font Face'] : 'sans-serif';
    const init_font_color_value = has_style ? selc_window['Window Font and Style Configuration']['Base Font Color'] : '#ffffff';
    const init_font_outline_color_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Color'] : 'rgba(0, 0, 0, 0.5)';
    const init_font_outline_thickness_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Thickness'] : 3;
    const init_window_skin_value = has_style ? selc_window['Window Font and Style Configuration']['Window Skin'] : "Window";
    const init_window_opacity_value = has_style ? selc_window['Window Font and Style Configuration']['Window Opacity'] : 255;
    const init_window_dimmer_value = has_style ? selc_window['Window Font and Style Configuration']['Show Window Dimmer'] : false;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Window Font and Style Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const style_config_container = document.createElement('div');
    style_config_container.id = "Button_Container";
    style_config_container.classList.add("content");
    container.appendChild(style_config_container);
    const font_size_label = document.createElement('label');
    font_size_label.id = "Window_Label";
    font_size_label.textContent = `Font Size: ${init_font_size_value}`;
    style_config_container.appendChild(font_size_label);
    const font_size_input = document.createElement('input');
    font_size_input.id = "Window_Input";
    font_size_input.setAttribute("size", 15);
    font_size_input.setAttribute("value", init_font_size_value);
    font_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Window Font and Style Configuration']['Font Size'] = value;
            font_size_label.textContent = `Font Size: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_size_input);
    const font_face_label = document.createElement('label');
    font_face_label.id = "Window_Label";
    font_face_label.textContent = `Font Face: ${init_font_face_value}`;
    style_config_container.appendChild(font_face_label);
    const font_face_input = document.createElement('input');
    font_face_input.id = "Window_Input";
    font_face_input.setAttribute("size", 15);
    font_face_input.setAttribute("value", init_font_face_value);
    font_face_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_face_input.value;
            menu['Actor Data Windows'][index]['Window Font and Style Configuration']['Font Face'] = value;
            font_face_label.textContent = `Font Face: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_face_input);
    const font_color_label = document.createElement('label');
    font_color_label.id = "Window_Label";
    font_color_label.textContent = `Base Font Color: ${init_font_color_value}`;
    style_config_container.appendChild(font_color_label);
    const font_color_input = document.createElement('input');
    font_color_input.id = "Window_Input";
    font_color_input.setAttribute("size", 15);
    font_color_input.setAttribute("value", init_font_color_value);
    font_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_color_input.value;
            if(value[0] != "#"){
                if(value.length > 0){
                    alert("You need to use color hex value.");
                }
            }else{
                menu['Actor Data Windows'][index]['Window Font and Style Configuration']['Base Font Color'] = value;
                font_color_label.textContent = `Base Font Color: ${value}`;
                recompileMenus(menu_data, menu);
            }
        }
    })
    style_config_container.appendChild(font_color_input);
    const font_outline_color_label = document.createElement('label');
    font_outline_color_label.id = "Window_Label";
    font_outline_color_label.textContent = `Font Outline Color: ${init_font_outline_color_value}`;
    style_config_container.appendChild(font_outline_color_label);
    const font_outline_color_input = document.createElement('input');
    font_outline_color_input.id = "Window_Input";
    font_outline_color_input.setAttribute("size", 15);
    font_outline_color_input.setAttribute("value", init_font_outline_color_value);
    font_outline_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_color_input.value;
            menu['Actor Data Windows'][index]['Window Font and Style Configuration']['Font Outline Color'] = value;
            font_outline_color_label.textContent = `Font Outline Color: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_color_input);
    const font_outline_size_label = document.createElement('label');
    font_outline_size_label.id = "Window_Label";
    font_outline_size_label.textContent = `Font Outline Thickness: ${init_font_outline_thickness_value}`;
    style_config_container.appendChild(font_outline_size_label);
    const font_outline_size_input = document.createElement('input');
    font_outline_size_input.id = "Window_Input";
    font_outline_size_input.setAttribute("size", 15);
    font_outline_size_input.setAttribute("value", init_font_outline_thickness_value);
    font_outline_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Window Font and Style Configuration']['Font Outline Thickness'] = value;
            font_outline_size_label.textContent = `Font Outline Thickness: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_size_input);
    const window_skin_label = document.createElement('label');
    window_skin_label.id = "Window_Label";
    window_skin_label.textContent = `Window Skin: ${init_window_skin_value}`;
    style_config_container.appendChild(window_skin_label);
    const window_skin_input = document.createElement('input');
    window_skin_input.id = "Window_Input";
    window_skin_input.setAttribute("size", 15);
    window_skin_input.setAttribute("type", "file");
    window_skin_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(window_skin_input.value){
            const input_paths = (window_skin_input.value || "").match(/(img\\system\\[a-z\d*|\W\D*]+)/gm);
            if(input_paths.length > 0){
                const path = input_paths[0];
                let file_name = path.replace("img\\system\\", "");
                file_name = file_name.replace(".png", "");
                file_name = file_name.replace("\\", "/");
                menu['Actor Data Windows'][index]['Window Font and Style Configuration']['Window Skin'] = file_name;
                window_skin_label.textContent = `Window Skin: ${file_name}`;
            }
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_skin_input);
    const window_opacity_label = document.createElement('label');
    window_opacity_label.id = "Window_Label";
    window_opacity_label.textContent = `Window Opacity: ${init_window_opacity_value}`;
    style_config_container.appendChild(window_opacity_label);
    const window_opacity_input = document.createElement('input');
    window_opacity_input.id = "Window_Input";
    window_opacity_input.setAttribute("size", 15);
    window_opacity_input.setAttribute("value", init_window_opacity_value);
    window_opacity_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_opacity_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Window Font and Style Configuration']['Window Opacity'] = value;
            window_opacity_label.textContent = `Window Opacity: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_opacity_input);
    const window_dimmer_label = document.createElement('label');
    window_dimmer_label.id = "Window_Label";
    window_dimmer_label.textContent = `Show Window Dimmer: `;
    style_config_container.appendChild(window_dimmer_label);
    const window_dimmer_input = document.createElement("input");
    window_dimmer_input.id = "Window_Check";
    window_dimmer_input.setAttribute("size", 15);
    window_dimmer_input.setAttribute("type", "checkbox");
    if(init_window_dimmer_value)window_dimmer_input.setAttribute("checked", true);
    window_dimmer_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_dimmer_input.checked;
            menu['Actor Data Windows'][index]['Window Font and Style Configuration']['Show Window Dimmer'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_dimmer_input);
}

const addWindowBasicStyleForm = function(container, data, index){
    const selc_windows = data['Basic Windows'];
    const selc_window = selc_windows[index];
    const has_style = !!selc_window['Window Font and Style Configuration'];
    const init_font_size_value = has_style ? selc_window['Window Font and Style Configuration']['Font Size'] : 16;
    const init_font_face_value = has_style ? selc_window['Window Font and Style Configuration']['Font Face'] : 'sans-serif';
    const init_font_color_value = has_style ? selc_window['Window Font and Style Configuration']['Base Font Color'] : '#ffffff';
    const init_font_outline_color_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Color'] : 'rgba(0, 0, 0, 0.5)';
    const init_font_outline_thickness_value = has_style ? selc_window['Window Font and Style Configuration']['Font Outline Thickness'] : 3;
    const init_window_skin_value = has_style ? selc_window['Window Font and Style Configuration']['Window Skin'] : "Window";
    const init_window_opacity_value = has_style ? selc_window['Window Font and Style Configuration']['Window Opacity'] : 255;
    const init_window_dimmer_value = has_style ? selc_window['Window Font and Style Configuration']['Show Window Dimmer'] : false;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Window Font and Style Configuration";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const style_config_container = document.createElement('div');
    style_config_container.id = "Button_Container";
    style_config_container.classList.add("content");
    container.appendChild(style_config_container);
    const font_size_label = document.createElement('label');
    font_size_label.id = "Window_Label";
    font_size_label.textContent = `Font Size: ${init_font_size_value}`;
    style_config_container.appendChild(font_size_label);
    const font_size_input = document.createElement('input');
    font_size_input.id = "Window_Input";
    font_size_input.setAttribute("size", 15);
    font_size_input.setAttribute("value", init_font_size_value);
    font_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Window Font and Style Configuration']['Font Size'] = value;
            font_size_label.textContent = `Font Size: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_size_input);
    const font_face_label = document.createElement('label');
    font_face_label.id = "Window_Label";
    font_face_label.textContent = `Font Face: ${init_font_face_value}`;
    style_config_container.appendChild(font_face_label);
    const font_face_input = document.createElement('input');
    font_face_input.id = "Window_Input";
    font_face_input.setAttribute("size", 15);
    font_face_input.setAttribute("value", init_font_face_value);
    font_face_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_face_input.value;
            menu['Basic Windows'][index]['Window Font and Style Configuration']['Font Face'] = value;
            font_face_label.textContent = `Font Face: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_face_input);
    const font_color_label = document.createElement('label');
    font_color_label.id = "Window_Label";
    font_color_label.textContent = `Base Font Color: ${init_font_color_value}`;
    style_config_container.appendChild(font_color_label);
    const font_color_input = document.createElement('input');
    font_color_input.id = "Window_Input";
    font_color_input.setAttribute("size", 15);
    font_color_input.setAttribute("value", init_font_color_value);
    font_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_color_input.value;
            if(value[0] != "#"){
                if(value.length > 0){
                    alert("You need to use color hex value.");
                }
            }else{
                menu['Basic Windows'][index]['Window Font and Style Configuration']['Base Font Color'] = value;
                font_color_label.textContent = `Base Font Color: ${value}`;
                recompileMenus(menu_data, menu);
            }
        }
    })
    style_config_container.appendChild(font_color_input);
    const font_outline_color_label = document.createElement('label');
    font_outline_color_label.id = "Window_Label";
    font_outline_color_label.textContent = `Font Outline Color: ${init_font_outline_color_value}`;
    style_config_container.appendChild(font_outline_color_label);
    const font_outline_color_input = document.createElement('input');
    font_outline_color_input.id = "Window_Input";
    font_outline_color_input.setAttribute("size", 15);
    font_outline_color_input.setAttribute("value", init_font_outline_color_value);
    font_outline_color_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_color_input.value;
            menu['Basic Windows'][index]['Window Font and Style Configuration']['Font Outline Color'] = value;
            font_outline_color_label.textContent = `Font Outline Color: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_color_input);
    const font_outline_size_label = document.createElement('label');
    font_outline_size_label.id = "Window_Label";
    font_outline_size_label.textContent = `Font Outline Thickness: ${init_font_outline_thickness_value}`;
    style_config_container.appendChild(font_outline_size_label);
    const font_outline_size_input = document.createElement('input');
    font_outline_size_input.id = "Window_Input";
    font_outline_size_input.setAttribute("size", 15);
    font_outline_size_input.setAttribute("value", init_font_outline_thickness_value);
    font_outline_size_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = font_outline_size_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Window Font and Style Configuration']['Font Outline Thickness'] = value;
            font_outline_size_label.textContent = `Font Outline Thickness: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(font_outline_size_input);
    const window_skin_label = document.createElement('label');
    window_skin_label.id = "Window_Label";
    window_skin_label.textContent = `Window Skin: ${init_window_skin_value}`;
    style_config_container.appendChild(window_skin_label);
    const window_skin_input = document.createElement('input');
    window_skin_input.id = "Window_Input";
    window_skin_input.setAttribute("size", 15);
    window_skin_input.setAttribute("type", "file");
    window_skin_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(window_skin_input.value){
            const input_paths = (window_skin_input.value || "").match(/(img\\system\\[a-z\d*|\W\D*]+)/gm);
            if(input_paths.length > 0){
                const path = input_paths[0];
                let file_name = path.replace("img\\system\\", "");
                file_name = file_name.replace(".png", "");
                file_name = file_name.replace("\\", "/");
                menu['Basic Windows'][index]['Window Font and Style Configuration']['Window Skin'] = file_name;
                window_skin_label.textContent = `Window Skin: ${file_name}`;
            }
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_skin_input);
    const window_opacity_label = document.createElement('label');
    window_opacity_label.id = "Window_Label";
    window_opacity_label.textContent = `Window Opacity: ${init_window_opacity_value}`;
    style_config_container.appendChild(window_opacity_label);
    const window_opacity_input = document.createElement('input');
    window_opacity_input.id = "Window_Input";
    window_opacity_input.setAttribute("size", 15);
    window_opacity_input.setAttribute("value", init_window_opacity_value);
    window_opacity_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_opacity_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Window Font and Style Configuration']['Window Opacity'] = value;
            window_opacity_label.textContent = `Window Opacity: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_opacity_input);
    const window_dimmer_label = document.createElement('label');
    window_dimmer_label.id = "Window_Label";
    window_dimmer_label.textContent = `Show Window Dimmer: `;
    style_config_container.appendChild(window_dimmer_label);
    const window_dimmer_input = document.createElement("input");
    window_dimmer_input.id = "Window_Check";
    window_dimmer_input.setAttribute("size", 15);
    window_dimmer_input.setAttribute("type", "checkbox");
    if(init_window_dimmer_value)window_dimmer_input.setAttribute("checked", true);
    window_dimmer_input.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = window_dimmer_input.checked;
            menu['Basic Windows'][index]['Window Font and Style Configuration']['Show Window Dimmer'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    style_config_container.appendChild(window_dimmer_input);
}

const addWindowDataRequirementsForm = function(container, data, index){
    const selc_windows = data['Selection Data Windows'];
    const selc_window = selc_windows[index];
    const requirements = selc_window['Display Requirements'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Display Requirements";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const requirements_config_container = document.createElement('div');
    requirements_config_container.id = "Button_Container";
    requirements_config_container.classList.add("content");
    container.appendChild(requirements_config_container);
    const swtch_label = document.createElement('label');
    swtch_label.id = "Window_Label";
    swtch_label.textContent = `Game Switch: ${requirements['Game Switch']}`;
    requirements_config_container.appendChild(swtch_label);
    const swtch_input = document.createElement("input");
    swtch_input.id = "Window_Input";
    swtch_input.setAttribute("value", requirements['Game Switch']);
    swtch_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = swtch_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Display Requirements']['Game Switch'] = value;
            swtch_label.textContent = `Game Switch: : ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(swtch_input);
    const var_label = document.createElement('label');
    var_label.id = "Window_Label";
    var_label.textContent = `Game Variable: ${requirements['Game Variable']}`;
    requirements_config_container.appendChild(var_label);
    const var_input = document.createElement("input");
    var_input.id = "Window_Input";
    var_input.setAttribute("value", requirements['Game Variable']);
    var_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = var_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Display Requirements']['Game Variable'] = value;
            var_label.textContent = `Game Variable: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(var_input);
    const var_min_label = document.createElement('label');
    var_min_label.id = "Window_Label";
    var_min_label.textContent = `Variable Minimum: ${requirements['Variable Minimum']}`;
    requirements_config_container.appendChild(var_min_label);
    const var_min_input = document.createElement("input");
    var_min_input.id = "Window_Input";
    var_min_input.setAttribute("value", requirements['Variable Minimum']);
    var_min_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = var_min_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Display Requirements']['Variable Minimum'] = value;
            var_min_label.textContent = `Variable Minimum: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(var_min_input);
    const var_max_label = document.createElement('label');
    var_max_label.id = "Window_Label";
    var_max_label.textContent = `Variable Maximum: ${requirements['Variable Maximum']}`;
    requirements_config_container.appendChild(var_max_label);
    const var_max_input = document.createElement("input");
    var_max_input.id = "Window_Input";
    var_max_input.setAttribute("value", requirements['Variable Maximum']);
    var_max_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = var_max_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Selection Data Windows'][index]['Display Requirements']['Variable Maximum'] = value;
            var_max_label.textContent = `Variable Maximum: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(var_max_input);
    const code_label = document.createElement('label');
    code_label.id = "Window_Label";
    code_label.textContent = `Code:`;
    requirements_config_container.appendChild(code_label);
    const code_input = document.createElement("input");
    code_input.id = "Window_Input";
    code_input.setAttribute("value", requirements['Code']);
    code_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = code_input.value;
            menu['Selection Data Windows'][index]['Display Requirements']['Code'] = value;
            code_label.textContent = `Code:`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(code_input);
}

const addActrWindowDataRequirementsForm = function(container, data, index){
    const selc_windows = data['Actor Data Windows'];
    const selc_window = selc_windows[index];
    const requirements = selc_window['Display Requirements'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Display Requirements";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Display_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const requirements_config_container = document.createElement('div');
    requirements_config_container.id = "Display_Config_Container";
    requirements_config_container.classList.add("content");
    container.appendChild(requirements_config_container);
    const swtch_label = document.createElement('label');
    swtch_label.id = "Window_Label";
    swtch_label.textContent = `Game Switch: ${requirements['Game Switch']}`;
    requirements_config_container.appendChild(swtch_label);
    const swtch_input = document.createElement("input");
    swtch_input.id = "Window_Input";
    swtch_input.setAttribute("value", requirements['Game Switch']);
    swtch_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = swtch_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Display Requirements']['Game Switch'] = value;
            swtch_label.textContent = `Game Switch: : ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(swtch_input);
    const var_label = document.createElement('label');
    var_label.id = "Window_Label";
    var_label.textContent = `Game Variable: ${requirements['Game Variable']}`;
    requirements_config_container.appendChild(var_label);
    const var_input = document.createElement("input");
    var_input.id = "Window_Input";
    var_input.setAttribute("value", requirements['Game Variable']);
    var_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = var_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Display Requirements']['Game Variable'] = value;
            var_label.textContent = `Game Variable: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(var_input);
    const var_min_label = document.createElement('label');
    var_min_label.id = "Window_Label";
    var_min_label.textContent = `Variable Minimum: ${requirements['Variable Minimum']}`;
    requirements_config_container.appendChild(var_min_label);
    const var_min_input = document.createElement("input");
    var_min_input.id = "Window_Input";
    var_min_input.setAttribute("value", requirements['Variable Minimum']);
    var_min_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = var_min_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Display Requirements']['Variable Minimum'] = value;
            var_min_label.textContent = `Variable Minimum: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(var_min_input);
    const var_max_label = document.createElement('label');
    var_max_label.id = "Window_Label";
    var_max_label.textContent = `Variable Maximum: ${requirements['Variable Maximum']}`;
    requirements_config_container.appendChild(var_max_label);
    const var_max_input = document.createElement("input");
    var_max_input.id = "Window_Input";
    var_max_input.setAttribute("value", requirements['Variable Maximum']);
    var_max_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = var_max_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Actor Data Windows'][index]['Display Requirements']['Variable Maximum'] = value;
            var_max_label.textContent = `Variable Maximum: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(var_max_input);
    const code_label = document.createElement('label');
    code_label.id = "Window_Label";
    code_label.textContent = `Code:`;
    requirements_config_container.appendChild(code_label);
    const code_input = document.createElement("input");
    code_input.id = "Window_Input";
    code_input.setAttribute("value", requirements['Code']);
    code_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = code_input.value;
            menu['Actor Data Windows'][index]['Display Requirements']['Code'] = value;
            code_label.textContent = `Code:`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(code_input);
}

const addWindowBasicRequirementsForm = function(container, data, index){
    const selc_windows = data['Basic Windows'];
    const selc_window = selc_windows[index];
    const requirements = selc_window['Display Requirements'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Display Requirements";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const requirements_config_container = document.createElement('div');
    requirements_config_container.id = "Button_Container";
    requirements_config_container.classList.add("content");
    container.appendChild(requirements_config_container);
    const swtch_label = document.createElement('label');
    swtch_label.id = "Window_Label";
    swtch_label.textContent = `Game Switch: ${requirements['Game Switch']}`;
    requirements_config_container.appendChild(swtch_label);
    const swtch_input = document.createElement("input");
    swtch_input.id = "Window_Input";
    swtch_input.setAttribute("value", requirements['Game Switch']);
    swtch_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = swtch_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Display Requirements']['Game Switch'] = value;
            swtch_label.textContent = `Game Switch: : ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(swtch_input);
    const var_label = document.createElement('label');
    var_label.id = "Window_Label";
    var_label.textContent = `Game Variable: ${requirements['Game Variable']}`;
    requirements_config_container.appendChild(var_label);
    const var_input = document.createElement("input");
    var_input.id = "Window_Input";
    var_input.setAttribute("value", requirements['Game Variable']);
    var_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = var_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Display Requirements']['Game Variable'] = value;
            var_label.textContent = `Game Variable: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(var_input);
    const var_min_label = document.createElement('label');
    var_min_label.id = "Window_Label";
    var_min_label.textContent = `Variable Minimum: ${requirements['Variable Minimum']}`;
    requirements_config_container.appendChild(var_min_label);
    const var_min_input = document.createElement("input");
    var_min_input.id = "Window_Input";
    var_min_input.setAttribute("value", requirements['Variable Minimum']);
    var_min_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = var_min_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Display Requirements']['Variable Minimum'] = value;
            var_min_label.textContent = `Variable Minimum: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(var_min_input);
    const var_max_label = document.createElement('label');
    var_max_label.id = "Window_Label";
    var_max_label.textContent = `Variable Maximum: ${requirements['Variable Maximum']}`;
    requirements_config_container.appendChild(var_max_label);
    const var_max_input = document.createElement("input");
    var_max_input.id = "Window_Input";
    var_max_input.setAttribute("value", requirements['Variable Maximum']);
    var_max_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = var_max_input.value;
            if(isNaN(eval(value)))value = 0;
            menu['Basic Windows'][index]['Display Requirements']['Variable Maximum'] = value;
            var_max_label.textContent = `Variable Maximum: ${value}`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(var_max_input);
    const code_label = document.createElement('label');
    code_label.id = "Window_Label";
    code_label.textContent = `Code:`;
    requirements_config_container.appendChild(code_label);
    const code_input = document.createElement("input");
    code_input.id = "Window_Input";
    code_input.setAttribute("value", requirements['Code']);
    code_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            let value = code_input.value;
            menu['Basic Windows'][index]['Display Requirements']['Code'] = value;
            code_label.textContent = `Code:`;
            recompileMenus(menu_data, menu);
        }
    })
    requirements_config_container.appendChild(code_input);
}

const addWindowGaugesForm = function(container, data){
    const selection_window = data['Selection Window'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Gauges";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const gauge_config_container = document.createElement('div');
    gauge_config_container.id = "Button_Container";
    gauge_config_container.classList.add("content");
    container.appendChild(gauge_config_container);
    const existing_gauges = selection_window['Gauges'] || [];
    let index = 0;
    existing_gauges.forEach((gauge_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${gauge_data['Label']}`;
        collapse_button._saved_index = JSON.parse(JSON.stringify(index));
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Gauge_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        gauge_config_container.appendChild(collapse_button);
        const gauge_div = document.createElement('div');
        gauge_div.id = "Gauge_Container";
        gauge_div.classList.add("content");
        gauge_config_container.appendChild(gauge_div);
        const gauge_label_label = document.createElement('label');
        gauge_label_label.id = "Window_Label";
        gauge_label_label.textContent = `Label: ${gauge_data['Label']}`;
        gauge_div.appendChild(gauge_label_label);
        const gauge_label_input = document.createElement('input');
        gauge_label_input.setAttribute("size", 25);
        gauge_label_input.setAttribute("value", gauge_data['Label']);
        gauge_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_label_input.value){
                    const new_label = gauge_label_input.value;
                    menu['Selection Window']['Gauges'][sav_index]['Label'] = new_label;
                    gauge_label_label.textContent = `Label: ${new_label}`;
                    gauge_data['Label'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_label_input);
        const gauge_x_label_label = document.createElement(`label`);
        gauge_x_label_label.id = "Window_Label";
        gauge_x_label_label.textContent = `Label X: ${gauge_data['Label X']}`;
        gauge_div.appendChild(gauge_x_label_label);
        const gauge_x_label_input = document.createElement("input");
        gauge_x_label_input.id = "Window_Input";
        gauge_x_label_input.setAttribute("value", gauge_data['Label X']);
        gauge_x_label_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_label_input.value){
                    let value = gauge_x_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Window']['Gauges'][sav_index]['Label X'] = value;
                    gauge_x_label_label.textContent = `Label X: ${value}`;
                    gauge_data['Label X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_label_input);
        const gauge_y_label_label = document.createElement(`label`);
        gauge_y_label_label.id = "Window_Label";
        gauge_y_label_label.textContent = `Label Y: ${gauge_data['Label Y']}`;
        gauge_div.appendChild(gauge_y_label_label);
        const gauge_y_label_input = document.createElement("input");
        gauge_y_label_input.id = "Window_Input";
        gauge_y_label_input.setAttribute("value", gauge_data['Label Y']);
        gauge_y_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_label_input.value){
                    let value = gauge_y_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Window']['Gauges'][sav_index]['Label Y'] = value;
                    gauge_y_label_label.textContent = `Label Y: ${value}`;
                    gauge_data['Label Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_label_input);
        const gauge_cur_val_label = document.createElement(`label`);
        gauge_cur_val_label.id = "Window_Label";
        gauge_cur_val_label.textContent = `Gauge Current Value: ${gauge_data['Gauge Current Value']}`;
        gauge_div.appendChild(gauge_cur_val_label);
        const gauge_cur_val_input = document.createElement("input");
        gauge_cur_val_input.id = "Window_Input";
        gauge_cur_val_input.setAttribute("value", gauge_data['Gauge Current Value'] || "");
        gauge_cur_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_cur_val_input.value){
                    let value = gauge_cur_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge Current Value'] = value;
                    gauge_cur_val_label.textContent = `Gauge Current Value: ${value}`;
                    gauge_data['Gauge Current Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_cur_val_input);
        const gauge_max_val_label = document.createElement(`label`);
        gauge_max_val_label.id = "Window_Label";
        gauge_max_val_label.textContent = `Gauge Max Value: ${gauge_data['Gauge Max Value']}`;
        gauge_div.appendChild(gauge_max_val_label);
        const gauge_max_val_input = document.createElement("input");
        gauge_max_val_input.id = "Window_Input";
        gauge_max_val_input.setAttribute("value", gauge_data['Gauge Max Value'] || "");
        gauge_max_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_max_val_input.value){
                    let value = gauge_max_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge Max Value'] = value;
                    gauge_max_val_label.textContent = `Gauge Max Value: ${value}`;
                    gauge_data['Gauge Max Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_max_val_input);
        const gauge_x_label = document.createElement('label');
        gauge_x_label.id = "Window_Label";
        gauge_x_label.textContent = `Gauge X: ${gauge_data['Gauge X']}`;
        gauge_div.appendChild(gauge_x_label);
        const gauge_x_input = document.createElement("input");
        gauge_x_input.id = "Window_Input";
        gauge_x_input.setAttribute("value", gauge_data['Gauge X'] || 0);
        gauge_x_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_input.value){
                    let value = gauge_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge X'] = value;
                    gauge_x_label.textContent = `Gauge X: ${value}`;
                    gauge_data['Gauge X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_input);
        const gauge_y_label = document.createElement('label');
        gauge_y_label.id = "Window_Label";
        gauge_y_label.textContent = `Gauge Y: ${gauge_data['Gauge Y']}`;
        gauge_div.appendChild(gauge_y_label);
        const gauge_y_input = document.createElement("input");
        gauge_y_input.id = "Window_Input";
        gauge_y_input.setAttribute("value", gauge_data['Gauge Y'] || 0);
        gauge_y_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_input.value){
                    let value = gauge_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge Y'] = value;
                    gauge_y_label.textContent = `Gauge Y: ${value}`;
                    gauge_data['Gauge Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_input);
        const gauge_w_label = document.createElement('label');
        gauge_w_label.id = "Window_Label";
        gauge_w_label.textContent = `Gauge Width: ${gauge_data['Gauge Width']}`;
        gauge_div.appendChild(gauge_w_label);
        const gauge_w_input = document.createElement("input");
        gauge_w_input.id = "Window_Input";
        gauge_w_input.setAttribute("value", gauge_data['Gauge Width'] || 0);
        gauge_w_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_w_input.value){
                    let value = gauge_w_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge Width'] = value;
                    gauge_w_label.textContent = `Gauge Width: ${value}`;
                    gauge_data['Gauge Width'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_w_input);
        const gauge_h_label = document.createElement('label');
        gauge_h_label.id = "Window_Label";
        gauge_h_label.textContent = `Gauge Height: ${gauge_data['Gauge Height']}`;
        gauge_div.appendChild(gauge_h_label);
        const gauge_h_input = document.createElement("input");
        gauge_h_input.id = "Window_Input";
        gauge_h_input.setAttribute("value", gauge_data['Gauge Height'] || 0);
        gauge_h_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_h_input.value){
                    let value = gauge_h_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge Height'] = value;
                    gauge_h_label.textContent = `Gauge Height: ${value}`;
                    gauge_data['Gauge Height'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_h_input);
        const gauge_b_label = document.createElement('label');
        gauge_b_label.id = "Window_Label";
        gauge_b_label.textContent = `Gauge Border: ${gauge_data['Gauge Border']}`;
        gauge_div.appendChild(gauge_b_label);
        const gauge_b_input = document.createElement("input");
        gauge_b_input.id = "Window_Input";
        gauge_b_input.setAttribute("value", gauge_data['Gauge Border'] || 0);
        gauge_b_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_b_input.value){
                    let value = gauge_b_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge Border'] = value;
                    gauge_b_label.textContent = `Gauge Border: ${value}`;
                    gauge_data['Gauge Border'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_b_input);
        const gauge_border_color_label = document.createElement('label');
        gauge_border_color_label.id = "Window_Label";
        gauge_border_color_label.textContent = `Gauge Border Color: ${gauge_data['Gauge Border Color']}`;
        gauge_div.appendChild(gauge_border_color_label);
        const gauge_border_color_input = document.createElement("input");
        gauge_border_color_input.id = "Window_Input";
        gauge_border_color_input.setAttribute("value", gauge_data['Gauge Border Color']);
        gauge_border_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_border_color_input.value){
                    let value = gauge_border_color_input.value;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge Border Color'] = value;
                    gauge_border_color_label.textContent = `Gauge Border Color: ${value}`;
                    gauge_data['Gauge Border Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_border_color_input);
        const gauge_background_color_label = document.createElement('label');
        gauge_background_color_label.id = "Window_Label";
        gauge_background_color_label.textContent = `Gauge Background Color: ${gauge_data['Gauge Background Color']}`;
        gauge_div.appendChild(gauge_background_color_label);
        const gauge_background_color_input = document.createElement("input");
        gauge_background_color_input.id = "Window_Input";
        gauge_background_color_input.setAttribute("value", gauge_data['Gauge Background Color']);
        gauge_background_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_background_color_input.value){
                    let value = gauge_background_color_input.value;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge Background Color'] = value;
                    gauge_background_color_label.textContent = `Gauge Background Color: ${value}`;
                    gauge_data['Gauge Background Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_background_color_input);
        const gauge_color_label = document.createElement('label');
        gauge_color_label.id = "Window_Label";
        gauge_color_label.textContent = `Gauge Color: ${gauge_data['Gauge Color']}`;
        gauge_div.appendChild(gauge_color_label);
        const gauge_color_input = document.createElement("input");
        gauge_color_input.id = "Window_Input";
        gauge_color_input.setAttribute("value", gauge_data['Gauge Color']);
        gauge_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_color_input.value){
                    let value = gauge_color_input.value;
                    menu['Selection Window']['Gauges'][sav_index]['Gauge Color'] = value;
                    gauge_color_label.textContent = `Gauge Color: ${value}`;
                    gauge_data['Gauge Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_color_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const del_index = collapse_button._saved_index;
            if(del_index >= 0){
                menu['Selection Window']['Gauges'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        gauge_div.appendChild(delete_button);
        index++;
    })
    const add_gauge_button = document.createElement('button');
    add_gauge_button.id = "Add_Button";
    add_gauge_button.type = 'button';
    add_gauge_button.textContent = "ADD GAUGE";
    add_gauge_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const gauges = menu['Selection Window']['Gauges'];
        const last_gauge = gauges[gauges.length - 1];
        let index = 0;
        if(last_gauge){
            index = eval(last_gauge['ID']) + 1;
        }
        const gauge_obj = {
            "ID":`${index}`,
            "Label":"",
            "Label X":"0",
            "Label Y":"0",
            "Gauge Current Value":"0",
            "Gauge Max Value":"0",
            "Gauge X":"0",
            "Gauge Y":"0",
            "Gauge Width":"0",
            "Gauge Height":"0",
            "Gauge Border":"0",
            "Gauge Border Color":"#000000",
            "Gauge Background Color":"#666666",
            "Gauge Color":"#aaffaa"
        }
        gauges.push(gauge_obj);
        recompileMenus(menu_data, menu, true);
    })
    gauge_config_container.appendChild(add_gauge_button);
}

const addWindowDataGaugesForm = function(container, data, index){
    const selection_windows = data['Selection Data Windows'];
    const selection_window = selection_windows[index];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Gauges";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const gauge_config_container = document.createElement('div');
    gauge_config_container.id = "Button_Container";
    gauge_config_container.classList.add("content");
    container.appendChild(gauge_config_container);
    const existing_gauges = selection_window['Gauges'] || [];
    let index_g = 0;
    existing_gauges.forEach((gauge_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${gauge_data['Label']}`;
        collapse_button._saved_index = JSON.parse(JSON.stringify(index_g));
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Gauge_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        gauge_config_container.appendChild(collapse_button);
        const gauge_div = document.createElement('div');
        gauge_div.id = "Gauge_Container";
        gauge_div.classList.add("content");
        gauge_config_container.appendChild(gauge_div);
        const gauge_label_label = document.createElement('label');
        gauge_label_label.id = "Window_Label";
        gauge_label_label.textContent = `Label: ${gauge_data['Label']}`;
        gauge_div.appendChild(gauge_label_label);
        const gauge_label_input = document.createElement('input');
        gauge_label_input.setAttribute("size", 25);
        gauge_label_input.setAttribute("value", gauge_data['Label']);
        gauge_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_label_input.value){
                    const new_label = gauge_label_input.value;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Label'] = new_label;
                    gauge_label_label.textContent = `Label: ${new_label}`;
                    gauge_data['Label'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_label_input);
        const gauge_x_label_label = document.createElement(`label`);
        gauge_x_label_label.id = "Window_Label";
        gauge_x_label_label.textContent = `Label X: ${gauge_data['Label X']}`;
        gauge_div.appendChild(gauge_x_label_label);
        const gauge_x_label_input = document.createElement("input");
        gauge_x_label_input.id = "Window_Input";
        gauge_x_label_input.setAttribute("value", gauge_data['Label X']);
        gauge_x_label_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_label_input.value){
                    let value = gauge_x_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Label X'] = value;
                    gauge_x_label_label.textContent = `Label X: ${value}`;
                    gauge_data['Label X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_label_input);
        const gauge_y_label_label = document.createElement(`label`);
        gauge_y_label_label.id = "Window_Label";
        gauge_y_label_label.textContent = `Label Y: ${gauge_data['Label Y']}`;
        gauge_div.appendChild(gauge_y_label_label);
        const gauge_y_label_input = document.createElement("input");
        gauge_y_label_input.id = "Window_Input";
        gauge_y_label_input.setAttribute("value", gauge_data['Label Y']);
        gauge_y_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_label_input.value){
                    let value = gauge_y_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Label Y'] = value;
                    gauge_y_label_label.textContent = `Label Y: ${value}`;
                    gauge_data['Label Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_label_input);
        const gauge_cur_val_label = document.createElement(`label`);
        gauge_cur_val_label.id = "Window_Label";
        gauge_cur_val_label.textContent = `Gauge Current Value: ${gauge_data['Gauge Current Value']}`;
        gauge_div.appendChild(gauge_cur_val_label);
        const gauge_cur_val_input = document.createElement("input");
        gauge_cur_val_input.id = "Window_Input";
        gauge_cur_val_input.setAttribute("value", gauge_data['Gauge Current Value'] || "");
        gauge_cur_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_cur_val_input.value){
                    let value = gauge_cur_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge Current Value'] = value;
                    gauge_cur_val_label.textContent = `Gauge Current Value: ${value}`;
                    gauge_data['Gauge Current Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_cur_val_input);
        const gauge_max_val_label = document.createElement(`label`);
        gauge_max_val_label.id = "Window_Label";
        gauge_max_val_label.textContent = `Gauge Max Value: ${gauge_data['Gauge Max Value']}`;
        gauge_div.appendChild(gauge_max_val_label);
        const gauge_max_val_input = document.createElement("input");
        gauge_max_val_input.id = "Window_Input";
        gauge_max_val_input.setAttribute("value", gauge_data['Gauge Max Value'] || "");
        gauge_max_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_max_val_input.value){
                    let value = gauge_max_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge Max Value'] = value;
                    gauge_max_val_label.textContent = `Gauge Max Value: ${value}`;
                    gauge_data['Gauge Max Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_max_val_input);
        const gauge_x_label = document.createElement('label');
        gauge_x_label.id = "Window_Label";
        gauge_x_label.textContent = `Gauge X: ${gauge_data['Gauge X']}`;
        gauge_div.appendChild(gauge_x_label);
        const gauge_x_input = document.createElement("input");
        gauge_x_input.id = "Window_Input";
        gauge_x_input.setAttribute("value", gauge_data['Gauge X'] || 0);
        gauge_x_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_input.value){
                    let value = gauge_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge X'] = value;
                    gauge_x_label.textContent = `Gauge X: ${value}`;
                    gauge_data['Gauge X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_input);
        const gauge_y_label = document.createElement('label');
        gauge_y_label.id = "Window_Label";
        gauge_y_label.textContent = `Gauge Y: ${gauge_data['Gauge Y']}`;
        gauge_div.appendChild(gauge_y_label);
        const gauge_y_input = document.createElement("input");
        gauge_y_input.id = "Window_Input";
        gauge_y_input.setAttribute("value", gauge_data['Gauge Y'] || 0);
        gauge_y_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_input.value){
                    let value = gauge_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge Y'] = value;
                    gauge_y_label.textContent = `Gauge Y: ${value}`;
                    gauge_data['Gauge Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_input);
        const gauge_w_label = document.createElement('label');
        gauge_w_label.id = "Window_Label";
        gauge_w_label.textContent = `Gauge Width: ${gauge_data['Gauge Width']}`;
        gauge_div.appendChild(gauge_w_label);
        const gauge_w_input = document.createElement("input");
        gauge_w_input.id = "Window_Input";
        gauge_w_input.setAttribute("value", gauge_data['Gauge Width'] || 0);
        gauge_w_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_w_input.value){
                    let value = gauge_w_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge Width'] = value;
                    gauge_w_label.textContent = `Gauge Width: ${value}`;
                    gauge_data['Gauge Width'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_w_input);
        const gauge_h_label = document.createElement('label');
        gauge_h_label.id = "Window_Label";
        gauge_h_label.textContent = `Gauge Height: ${gauge_data['Gauge Height']}`;
        gauge_div.appendChild(gauge_h_label);
        const gauge_h_input = document.createElement("input");
        gauge_h_input.id = "Window_Input";
        gauge_h_input.setAttribute("value", gauge_data['Gauge Height'] || 0);
        gauge_h_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_h_input.value){
                    let value = gauge_h_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge Height'] = value;
                    gauge_h_label.textContent = `Gauge Height: ${value}`;
                    gauge_data['Gauge Height'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_h_input);
        const gauge_b_label = document.createElement('label');
        gauge_b_label.id = "Window_Label";
        gauge_b_label.textContent = `Gauge Border: ${gauge_data['Gauge Border']}`;
        gauge_div.appendChild(gauge_b_label);
        const gauge_b_input = document.createElement("input");
        gauge_b_input.id = "Window_Input";
        gauge_b_input.setAttribute("value", gauge_data['Gauge Border'] || 0);
        gauge_b_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_b_input.value){
                    let value = gauge_b_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge Border'] = value;
                    gauge_b_label.textContent = `Gauge Border: ${value}`;
                    gauge_data['Gauge Border'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_b_input);
        const gauge_border_color_label = document.createElement('label');
        gauge_border_color_label.id = "Window_Label";
        gauge_border_color_label.textContent = `Gauge Border Color: ${gauge_data['Gauge Border Color']}`;
        gauge_div.appendChild(gauge_border_color_label);
        const gauge_border_color_input = document.createElement("input");
        gauge_border_color_input.id = "Window_Input";
        gauge_border_color_input.setAttribute("value", gauge_data['Gauge Border Color']);
        gauge_border_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_border_color_input.value){
                    let value = gauge_border_color_input.value;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge Border Color'] = value;
                    gauge_border_color_label.textContent = `Gauge Border Color: ${value}`;
                    gauge_data['Gauge Border Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_border_color_input);
        const gauge_background_color_label = document.createElement('label');
        gauge_background_color_label.id = "Window_Label";
        gauge_background_color_label.textContent = `Gauge Background Color: ${gauge_data['Gauge Background Color']}`;
        gauge_div.appendChild(gauge_background_color_label);
        const gauge_background_color_input = document.createElement("input");
        gauge_background_color_input.id = "Window_Input";
        gauge_background_color_input.setAttribute("value", gauge_data['Gauge Background Color']);
        gauge_background_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_background_color_input.value){
                    let value = gauge_background_color_input.value;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge Background Color'] = value;
                    gauge_background_color_label.textContent = `Gauge Background Color: ${value}`;
                    gauge_data['Gauge Background Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_background_color_input);
        const gauge_color_label = document.createElement('label');
        gauge_color_label.id = "Window_Label";
        gauge_color_label.textContent = `Gauge Color: ${gauge_data['Gauge Color']}`;
        gauge_div.appendChild(gauge_color_label);
        const gauge_color_input = document.createElement("input");
        gauge_color_input.id = "Window_Input";
        gauge_color_input.setAttribute("value", gauge_data['Gauge Color']);
        gauge_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_color_input.value){
                    let value = gauge_color_input.value;
                    menu['Selection Data Windows'][index]['Gauges'][sav_index]['Gauge Color'] = value;
                    gauge_color_label.textContent = `Gauge Color: ${value}`;
                    gauge_data['Gauge Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_color_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const del_index = collapse_button._saved_index;
            if(del_index >= 0){
                menu['Selection Data Windows'][index]['Gauges'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        gauge_div.appendChild(delete_button);
        index_g++;
    })
    const add_gauge_button = document.createElement('button');
    add_gauge_button.id = "Add_Button";
    add_gauge_button.type = 'button';
    add_gauge_button.textContent = "ADD GAUGE";
    add_gauge_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const gauges = menu['Selection Data Windows'][index]['Gauges'];
        const last_gauge = gauges[gauges.length - 1];
        let new_index = 0;
        if(last_gauge){
            new_index = eval(last_gauge['ID']) + 1;
        }
        const gauge_obj = {
            "ID":`${new_index}`,
            "Label":"",
            "Label X":"0",
            "Label Y":"0",
            "Gauge Current Value":"0",
            "Gauge Max Value":"0",
            "Gauge X":"0",
            "Gauge Y":"0",
            "Gauge Width":"0",
            "Gauge Height":"0",
            "Gauge Border":"0",
            "Gauge Border Color":"#000000",
            "Gauge Background Color":"#666666",
            "Gauge Color":"#aaffaa"
        }
        gauges.push(gauge_obj);
        recompileMenus(menu_data, menu, true);
    })
    gauge_config_container.appendChild(add_gauge_button);
}

const addActrSelcWindowGaugesForm = function(container, data){
    const selection_window = data['Actor Selection Window'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Gauges";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const gauge_config_container = document.createElement('div');
    gauge_config_container.id = "Button_Container";
    gauge_config_container.classList.add("content");
    container.appendChild(gauge_config_container);
    const existing_gauges = selection_window['Gauges'] || [];
    let index = 0;
    existing_gauges.forEach((gauge_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${gauge_data['Label']}`;
        collapse_button._saved_index = JSON.parse(JSON.stringify(index));
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Gauge_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        gauge_config_container.appendChild(collapse_button);
        const gauge_div = document.createElement('div');
        gauge_div.id = "Gauge_Container";
        gauge_div.classList.add("content");
        gauge_config_container.appendChild(gauge_div);
        const gauge_label_label = document.createElement('label');
        gauge_label_label.id = "Window_Label";
        gauge_label_label.textContent = `Label: ${gauge_data['Label']}`;
        gauge_div.appendChild(gauge_label_label);
        const gauge_label_input = document.createElement('input');
        gauge_label_input.setAttribute("size", 25);
        gauge_label_input.setAttribute("value", gauge_data['Label']);
        gauge_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_label_input.value){
                    const new_label = gauge_label_input.value;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Label'] = new_label;
                    gauge_label_label.textContent = `Label: ${new_label}`;
                    gauge_data['Label'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_label_input);
        const gauge_x_label_label = document.createElement(`label`);
        gauge_x_label_label.id = "Window_Label";
        gauge_x_label_label.textContent = `Label X: ${gauge_data['Label X']}`;
        gauge_div.appendChild(gauge_x_label_label);
        const gauge_x_label_input = document.createElement("input");
        gauge_x_label_input.id = "Window_Input";
        gauge_x_label_input.setAttribute("value", gauge_data['Label X']);
        gauge_x_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_label_input.value){
                    let value = gauge_x_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Label X'] = value;
                    gauge_x_label_label.textContent = `Label X: ${value}`;
                    gauge_data['Label X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_label_input);
        const gauge_y_label_label = document.createElement(`label`);
        gauge_y_label_label.id = "Window_Label";
        gauge_y_label_label.textContent = `Label Y: ${gauge_data['Label Y']}`;
        gauge_div.appendChild(gauge_y_label_label);
        const gauge_y_label_input = document.createElement("input");
        gauge_y_label_input.id = "Window_Input";
        gauge_y_label_input.setAttribute("value", gauge_data['Label Y']);
        gauge_y_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_label_input.value){
                    let value = gauge_y_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Label Y'] = value;
                    gauge_y_label_label.textContent = `Label Y: ${value}`;
                    gauge_data['Label Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_label_input);
        const gauge_cur_val_label = document.createElement(`label`);
        gauge_cur_val_label.id = "Window_Label";
        gauge_cur_val_label.textContent = `Gauge Current Value: ${gauge_data['Gauge Current Value']}`;
        gauge_div.appendChild(gauge_cur_val_label);
        const gauge_cur_val_input = document.createElement("input");
        gauge_cur_val_input.id = "Window_Input";
        gauge_cur_val_input.setAttribute("value", gauge_data['Gauge Current Value'] || "");
        gauge_cur_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_cur_val_input.value){
                    let value = gauge_cur_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge Current Value'] = value;
                    gauge_cur_val_label.textContent = `Gauge Current Value: ${value}`;
                    gauge_data['Gauge Current Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_cur_val_input);
        const gauge_max_val_label = document.createElement(`label`);
        gauge_max_val_label.id = "Window_Label";
        gauge_max_val_label.textContent = `Gauge Max Value: ${gauge_data['Gauge Max Value']}`;
        gauge_div.appendChild(gauge_max_val_label);
        const gauge_max_val_input = document.createElement("input");
        gauge_max_val_input.id = "Window_Input";
        gauge_max_val_input.setAttribute("value", gauge_data['Gauge Max Value'] || "");
        gauge_max_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_max_val_input.value){
                    let value = gauge_max_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge Max Value'] = value;
                    gauge_max_val_label.textContent = `Gauge Max Value: ${value}`;
                    gauge_data['Gauge Max Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_max_val_input);
        const gauge_x_label = document.createElement('label');
        gauge_x_label.id = "Window_Label";
        gauge_x_label.textContent = `Gauge X: ${gauge_data['Gauge X']}`;
        gauge_div.appendChild(gauge_x_label);
        const gauge_x_input = document.createElement("input");
        gauge_x_input.id = "Window_Input";
        gauge_x_input.setAttribute("value", gauge_data['Gauge X'] || 0);
        gauge_x_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_input.value){
                    let value = gauge_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge X'] = value;
                    gauge_x_label.textContent = `Gauge X: ${value}`;
                    gauge_data['Gauge X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_input);
        const gauge_y_label = document.createElement('label');
        gauge_y_label.id = "Window_Label";
        gauge_y_label.textContent = `Gauge Y: ${gauge_data['Gauge Y']}`;
        gauge_div.appendChild(gauge_y_label);
        const gauge_y_input = document.createElement("input");
        gauge_y_input.id = "Window_Input";
        gauge_y_input.setAttribute("value", gauge_data['Gauge Y'] || 0);
        gauge_y_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_input.value){
                    let value = gauge_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge Y'] = value;
                    gauge_y_label.textContent = `Gauge Y: ${value}`;
                    gauge_data['Gauge Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_input);
        const gauge_w_label = document.createElement('label');
        gauge_w_label.id = "Window_Label";
        gauge_w_label.textContent = `Gauge Width: ${gauge_data['Gauge Width']}`;
        gauge_div.appendChild(gauge_w_label);
        const gauge_w_input = document.createElement("input");
        gauge_w_input.id = "Window_Input";
        gauge_w_input.setAttribute("value", gauge_data['Gauge Width'] || 0);
        gauge_w_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_w_input.value){
                    let value = gauge_w_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge Width'] = value;
                    gauge_w_label.textContent = `Gauge Width: ${value}`;
                    gauge_data['Gauge Width'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_w_input);
        const gauge_h_label = document.createElement('label');
        gauge_h_label.id = "Window_Label";
        gauge_h_label.textContent = `Gauge Height: ${gauge_data['Gauge Height']}`;
        gauge_div.appendChild(gauge_h_label);
        const gauge_h_input = document.createElement("input");
        gauge_h_input.id = "Window_Input";
        gauge_h_input.setAttribute("value", gauge_data['Gauge Height'] || 0);
        gauge_h_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_h_input.value){
                    let value = gauge_h_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge Height'] = value;
                    gauge_h_label.textContent = `Gauge Height: ${value}`;
                    gauge_data['Gauge Height'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_h_input);
        const gauge_b_label = document.createElement('label');
        gauge_b_label.id = "Window_Label";
        gauge_b_label.textContent = `Gauge Border: ${gauge_data['Gauge Border']}`;
        gauge_div.appendChild(gauge_b_label);
        const gauge_b_input = document.createElement("input");
        gauge_b_input.id = "Window_Input";
        gauge_b_input.setAttribute("value", gauge_data['Gauge Border'] || 0);
        gauge_b_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_b_input.value){
                    let value = gauge_b_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge Border'] = value;
                    gauge_b_label.textContent = `Gauge Border: ${value}`;
                    gauge_data['Gauge Border'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_b_input);
        const gauge_border_color_label = document.createElement('label');
        gauge_border_color_label.id = "Window_Label";
        gauge_border_color_label.textContent = `Gauge Border Color: ${gauge_data['Gauge Border Color']}`;
        gauge_div.appendChild(gauge_border_color_label);
        const gauge_border_color_input = document.createElement("input");
        gauge_border_color_input.id = "Window_Input";
        gauge_border_color_input.setAttribute("value", gauge_data['Gauge Border Color']);
        gauge_border_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_border_color_input.value){
                    let value = gauge_border_color_input.value;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge Border Color'] = value;
                    gauge_border_color_label.textContent = `Gauge Border Color: ${value}`;
                    gauge_data['Gauge Border Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_border_color_input);
        const gauge_background_color_label = document.createElement('label');
        gauge_background_color_label.id = "Window_Label";
        gauge_background_color_label.textContent = `Gauge Background Color: ${gauge_data['Gauge Background Color']}`;
        gauge_div.appendChild(gauge_background_color_label);
        const gauge_background_color_input = document.createElement("input");
        gauge_background_color_input.id = "Window_Input";
        gauge_background_color_input.setAttribute("value", gauge_data['Gauge Background Color']);
        gauge_background_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_background_color_input.value){
                    let value = gauge_background_color_input.value;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge Background Color'] = value;
                    gauge_background_color_label.textContent = `Gauge Background Color: ${value}`;
                    gauge_data['Gauge Background Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_background_color_input);
        const gauge_color_label = document.createElement('label');
        gauge_color_label.id = "Window_Label";
        gauge_color_label.textContent = `Gauge Color: ${gauge_data['Gauge Color']}`;
        gauge_div.appendChild(gauge_color_label);
        const gauge_color_input = document.createElement("input");
        gauge_color_input.id = "Window_Input";
        gauge_color_input.setAttribute("value", gauge_data['Gauge Color']);
        gauge_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_color_input.value){
                    let value = gauge_color_input.value;
                    menu['Actor Selection Window']['Gauges'][sav_index]['Gauge Color'] = value;
                    gauge_color_label.textContent = `Gauge Color: ${value}`;
                    gauge_data['Gauge Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_color_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const del_index = collapse_button._saved_index;
            if(del_index >= 0){
                menu['Actor Selection Window']['Gauges'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        gauge_div.appendChild(delete_button);
        index++;
    })
    const add_gauge_button = document.createElement('button');
    add_gauge_button.id = "Add_Button";
    add_gauge_button.type = 'button';
    add_gauge_button.textContent = "ADD GAUGE";
    add_gauge_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const gauges = menu['Actor Selection Window']['Gauges'];
        const last_gauge = gauges[gauges.length - 1];
        let index = 0;
        if(last_gauge){
            index = eval(last_gauge['ID']) + 1;
        }
        const gauge_obj = {
            "ID":`${index}`,
            "Label":"",
            "Label X":"0",
            "Label Y":"0",
            "Gauge Current Value":"0",
            "Gauge Max Value":"0",
            "Gauge X":"0",
            "Gauge Y":"0",
            "Gauge Width":"0",
            "Gauge Height":"0",
            "Gauge Border":"0",
            "Gauge Border Color":"#000000",
            "Gauge Background Color":"#666666",
            "Gauge Color":"#aaffaa"
        }
        gauges.push(gauge_obj);
        recompileMenus(menu_data, menu, true);
    })
    gauge_config_container.appendChild(add_gauge_button);
}

const addActrDataWindowGaugesForm = function(container, data, index){
    const selection_windows = data['Actor Data Windows'];
    const selection_window = selection_windows[index];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Gauges";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const gauge_config_container = document.createElement('div');
    gauge_config_container.id = "Button_Container";
    gauge_config_container.classList.add("content");
    container.appendChild(gauge_config_container);
    const existing_gauges = selection_window['Gauges'] || [];
    let index_g = 0;
    existing_gauges.forEach((gauge_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${gauge_data['Label']}`;
        collapse_button._saved_index = JSON.parse(JSON.stringify(index_g));
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Gauge_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        gauge_config_container.appendChild(collapse_button);
        const gauge_div = document.createElement('div');
        gauge_div.id = "Gauge_Container";
        gauge_div.classList.add("content");
        gauge_config_container.appendChild(gauge_div);
        const gauge_label_label = document.createElement('label');
        gauge_label_label.id = "Window_Label";
        gauge_label_label.textContent = `Label: ${gauge_data['Label']}`;
        gauge_div.appendChild(gauge_label_label);
        const gauge_label_input = document.createElement('input');
        gauge_label_input.setAttribute("size", 25);
        gauge_label_input.setAttribute("value", gauge_data['Label']);
        gauge_label_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_label_input.value){
                    const new_label = gauge_label_input.value;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Label'] = new_label;
                    gauge_label_label.textContent = `Label: ${new_label}`;
                    gauge_data['Label'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_label_input);
        const gauge_x_label_label = document.createElement(`label`);
        gauge_x_label_label.id = "Window_Label";
        gauge_x_label_label.textContent = `Label X: ${gauge_data['Label X']}`;
        gauge_div.appendChild(gauge_x_label_label);
        const gauge_x_label_input = document.createElement("input");
        gauge_x_label_input.id = "Window_Input";
        gauge_x_label_input.setAttribute("value", gauge_data['Label X']);
        gauge_x_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_label_input.value){
                    let value = gauge_x_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Label X'] = value;
                    gauge_x_label_label.textContent = `Label X: ${value}`;
                    gauge_data['Label X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_label_input);
        const gauge_y_label_label = document.createElement(`label`);
        gauge_y_label_label.id = "Window_Label";
        gauge_y_label_label.textContent = `Label Y: ${gauge_data['Label Y']}`;
        gauge_div.appendChild(gauge_y_label_label);
        const gauge_y_label_input = document.createElement("input");
        gauge_y_label_input.id = "Window_Input";
        gauge_y_label_input.setAttribute("value", gauge_data['Label Y']);
        gauge_y_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_label_input.value){
                    let value = gauge_y_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Label Y'] = value;
                    gauge_y_label_label.textContent = `Label Y: ${value}`;
                    gauge_data['Label Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_label_input);
        const gauge_cur_val_label = document.createElement(`label`);
        gauge_cur_val_label.id = "Window_Label";
        gauge_cur_val_label.textContent = `Gauge Current Value: ${gauge_data['Gauge Current Value']}`;
        gauge_div.appendChild(gauge_cur_val_label);
        const gauge_cur_val_input = document.createElement("input");
        gauge_cur_val_input.id = "Window_Input";
        gauge_cur_val_input.setAttribute("value", gauge_data['Gauge Current Value'] || "");
        gauge_cur_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_cur_val_input.value){
                    let value = gauge_cur_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge Current Value'] = value;
                    gauge_cur_val_label.textContent = `Gauge Current Value: ${value}`;
                    gauge_data['Gauge Current Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_cur_val_input);
        const gauge_max_val_label = document.createElement(`label`);
        gauge_max_val_label.id = "Window_Label";
        gauge_max_val_label.textContent = `Gauge Max Value: ${gauge_data['Gauge Max Value']}`;
        gauge_div.appendChild(gauge_max_val_label);
        const gauge_max_val_input = document.createElement("input");
        gauge_max_val_input.id = "Window_Input";
        gauge_max_val_input.setAttribute("value", gauge_data['Gauge Max Value'] || "");
        gauge_max_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_max_val_input.value){
                    let value = gauge_max_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge Max Value'] = value;
                    gauge_max_val_label.textContent = `Gauge Max Value: ${value}`;
                    gauge_data['Gauge Max Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_max_val_input);
        const gauge_x_label = document.createElement('label');
        gauge_x_label.id = "Window_Label";
        gauge_x_label.textContent = `Gauge X: ${gauge_data['Gauge X']}`;
        gauge_div.appendChild(gauge_x_label);
        const gauge_x_input = document.createElement("input");
        gauge_x_input.id = "Window_Input";
        gauge_x_input.setAttribute("value", gauge_data['Gauge X'] || 0);
        gauge_x_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_input.value){
                    let value = gauge_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge X'] = value;
                    gauge_x_label.textContent = `Gauge X: ${value}`;
                    gauge_data['Gauge X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_input);
        const gauge_y_label = document.createElement('label');
        gauge_y_label.id = "Window_Label";
        gauge_y_label.textContent = `Gauge Y: ${gauge_data['Gauge Y']}`;
        gauge_div.appendChild(gauge_y_label);
        const gauge_y_input = document.createElement("input");
        gauge_y_input.id = "Window_Input";
        gauge_y_input.setAttribute("value", gauge_data['Gauge Y'] || 0);
        gauge_y_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_input.value){
                    let value = gauge_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge Y'] = value;
                    gauge_y_label.textContent = `Gauge Y: ${value}`;
                    gauge_data['Gauge Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_input);
        const gauge_w_label = document.createElement('label');
        gauge_w_label.id = "Window_Label";
        gauge_w_label.textContent = `Gauge Width: ${gauge_data['Gauge Width']}`;
        gauge_div.appendChild(gauge_w_label);
        const gauge_w_input = document.createElement("input");
        gauge_w_input.id = "Window_Input";
        gauge_w_input.setAttribute("value", gauge_data['Gauge Width'] || 0);
        gauge_w_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_w_input.value){
                    let value = gauge_w_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge Width'] = value;
                    gauge_w_label.textContent = `Gauge Width: ${value}`;
                    gauge_data['Gauge Width'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_w_input);
        const gauge_h_label = document.createElement('label');
        gauge_h_label.id = "Window_Label";
        gauge_h_label.textContent = `Gauge Height: ${gauge_data['Gauge Height']}`;
        gauge_div.appendChild(gauge_h_label);
        const gauge_h_input = document.createElement("input");
        gauge_h_input.id = "Window_Input";
        gauge_h_input.setAttribute("value", gauge_data['Gauge Height'] || 0);
        gauge_h_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_h_input.value){
                    let value = gauge_h_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge Height'] = value;
                    gauge_h_label.textContent = `Gauge Height: ${value}`;
                    gauge_data['Gauge Height'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_h_input);
        const gauge_b_label = document.createElement('label');
        gauge_b_label.id = "Window_Label";
        gauge_b_label.textContent = `Gauge Border: ${gauge_data['Gauge Border']}`;
        gauge_div.appendChild(gauge_b_label);
        const gauge_b_input = document.createElement("input");
        gauge_b_input.id = "Window_Input";
        gauge_b_input.setAttribute("value", gauge_data['Gauge Border'] || 0);
        gauge_b_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_b_input.value){
                    let value = gauge_b_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge Border'] = value;
                    gauge_b_label.textContent = `Gauge Border: ${value}`;
                    gauge_data['Gauge Border'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_b_input);
        const gauge_border_color_label = document.createElement('label');
        gauge_border_color_label.id = "Window_Label";
        gauge_border_color_label.textContent = `Gauge Border Color: ${gauge_data['Gauge Border Color']}`;
        gauge_div.appendChild(gauge_border_color_label);
        const gauge_border_color_input = document.createElement("input");
        gauge_border_color_input.id = "Window_Input";
        gauge_border_color_input.setAttribute("value", gauge_data['Gauge Border Color']);
        gauge_border_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_border_color_input.value){
                    let value = gauge_border_color_input.value;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge Border Color'] = value;
                    gauge_border_color_label.textContent = `Gauge Border Color: ${value}`;
                    gauge_data['Gauge Border Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_border_color_input);
        const gauge_background_color_label = document.createElement('label');
        gauge_background_color_label.id = "Window_Label";
        gauge_background_color_label.textContent = `Gauge Background Color: ${gauge_data['Gauge Background Color']}`;
        gauge_div.appendChild(gauge_background_color_label);
        const gauge_background_color_input = document.createElement("input");
        gauge_background_color_input.id = "Window_Input";
        gauge_background_color_input.setAttribute("value", gauge_data['Gauge Background Color']);
        gauge_background_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_background_color_input.value){
                    let value = gauge_background_color_input.value;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge Background Color'] = value;
                    gauge_background_color_label.textContent = `Gauge Background Color: ${value}`;
                    gauge_data['Gauge Background Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_background_color_input);
        const gauge_color_label = document.createElement('label');
        gauge_color_label.id = "Window_Label";
        gauge_color_label.textContent = `Gauge Color: ${gauge_data['Gauge Color']}`;
        gauge_div.appendChild(gauge_color_label);
        const gauge_color_input = document.createElement("input");
        gauge_color_input.id = "Window_Input";
        gauge_color_input.setAttribute("value", gauge_data['Gauge Color']);
        gauge_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_color_input.value){
                    let value = gauge_color_input.value;
                    menu['Actor Data Windows'][index]['Gauges'][sav_index]['Gauge Color'] = value;
                    gauge_color_label.textContent = `Gauge Color: ${value}`;
                    gauge_data['Gauge Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_color_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const del_index = collapse_button._saved_index;
            if(del_index >= 0){
                menu['Actor Data Windows'][index]['Gauges'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        gauge_div.appendChild(delete_button);
        index_g++;
    })
    const add_gauge_button = document.createElement('button');
    add_gauge_button.id = "Add_Button";
    add_gauge_button.type = 'button';
    add_gauge_button.textContent = "ADD GAUGE";
    add_gauge_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const gauges = menu['Actor Data Windows'][index]['Gauges'];
        const last_gauge = gauges[gauges.length - 1];
        let new_index = 0;
        if(last_gauge){
            new_index = eval(last_gauge['ID']) + 1;
        }
        const gauge_obj = {
            "ID":`${new_index}`,
            "Label":"",
            "Label X":"0",
            "Label Y":"0",
            "Gauge Current Value":"0",
            "Gauge Max Value":"0",
            "Gauge X":"0",
            "Gauge Y":"0",
            "Gauge Width":"0",
            "Gauge Height":"0",
            "Gauge Border":"0",
            "Gauge Border Color":"#000000",
            "Gauge Background Color":"#666666",
            "Gauge Color":"#aaffaa"
        }
        gauges.push(gauge_obj);
        recompileMenus(menu_data, menu, true);
    })
    gauge_config_container.appendChild(add_gauge_button);
}

const addWindowBasicGaugesForm = function(container, data, index){
    const selection_windows = data['Basic Windows'];
    const selection_window = selection_windows[index];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Gauges";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const gauge_config_container = document.createElement('div');
    gauge_config_container.id = "Button_Container";
    gauge_config_container.classList.add("content");
    container.appendChild(gauge_config_container);
    const existing_gauges = selection_window['Gauges'] || [];
    let index_g = 0;
    existing_gauges.forEach((gauge_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${gauge_data['Label']}`;
        collapse_button._saved_index = JSON.parse(JSON.stringify(index_g));
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Gauge_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        gauge_config_container.appendChild(collapse_button);
        const gauge_div = document.createElement('div');
        gauge_div.id = "Gauge_Container";
        gauge_div.classList.add("content");
        gauge_config_container.appendChild(gauge_div);
        const gauge_label_label = document.createElement('label');
        gauge_label_label.id = "Window_Label";
        gauge_label_label.textContent = `Label: ${gauge_data['Label']}`;
        gauge_div.appendChild(gauge_label_label);
        const gauge_label_input = document.createElement('input');
        gauge_label_input.id = "Window_Input"
        gauge_label_input.setAttribute("size", 25);
        gauge_label_input.setAttribute("value", gauge_data['Label']);
        gauge_label_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_label_input.value){
                    const new_label = gauge_label_input.value;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Label'] = new_label;
                    gauge_label_label.textContent = `Label: ${new_label}`;
                    gauge_data['Label'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_label_input);
        const gauge_x_label_label = document.createElement(`label`);
        gauge_x_label_label.id = "Window_Label";
        gauge_x_label_label.textContent = `Label X: ${gauge_data['Label X']}`;
        gauge_div.appendChild(gauge_x_label_label);
        const gauge_x_label_input = document.createElement("input");
        gauge_x_label_input.id = "Window_Input";
        gauge_x_label_input.setAttribute("value", gauge_data['Label X']);
        gauge_x_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_label_input.value){
                    let value = gauge_x_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Label X'] = value;
                    gauge_x_label_label.textContent = `Label X: ${value}`;
                    gauge_data['Label X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_label_input);
        const gauge_y_label_label = document.createElement(`label`);
        gauge_y_label_label.id = "Window_Label";
        gauge_y_label_label.textContent = `Label Y: ${gauge_data['Label Y']}`;
        gauge_div.appendChild(gauge_y_label_label);
        const gauge_y_label_input = document.createElement("input");
        gauge_y_label_input.id = "Window_Input";
        gauge_y_label_input.setAttribute("value", gauge_data['Label Y']);
        gauge_y_label_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_label_input.value){
                    let value = gauge_y_label_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Label Y'] = value;
                    gauge_y_label_label.textContent = `Label Y: ${value}`;
                    gauge_data['Label Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_label_input);
        const gauge_cur_val_label = document.createElement(`label`);
        gauge_cur_val_label.id = "Window_Label";
        gauge_cur_val_label.textContent = `Gauge Current Value: ${gauge_data['Gauge Current Value']}`;
        gauge_div.appendChild(gauge_cur_val_label);
        const gauge_cur_val_input = document.createElement("input");
        gauge_cur_val_input.id = "Window_Input";
        gauge_cur_val_input.setAttribute("value", gauge_data['Gauge Current Value'] || "");
        gauge_cur_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_cur_val_input.value){
                    let value = gauge_cur_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge Current Value'] = value;
                    gauge_cur_val_label.textContent = `Gauge Current Value: ${value}`;
                    gauge_data['Gauge Current Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_cur_val_input);
        const gauge_max_val_label = document.createElement(`label`);
        gauge_max_val_label.id = "Window_Label";
        gauge_max_val_label.textContent = `Gauge Max Value: ${gauge_data['Gauge Max Value']}`;
        gauge_div.appendChild(gauge_max_val_label);
        const gauge_max_val_input = document.createElement("input");
        gauge_max_val_input.id = "Window_Input";
        gauge_max_val_input.setAttribute("value", gauge_data['Gauge Max Value'] || "");
        gauge_max_val_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_max_val_input.value){
                    let value = gauge_max_val_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge Max Value'] = value;
                    gauge_max_val_label.textContent = `Gauge Max Value: ${value}`;
                    gauge_data['Gauge Max Value'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_max_val_input);
        const gauge_x_label = document.createElement('label');
        gauge_x_label.id = "Window_Label";
        gauge_x_label.textContent = `Gauge X: ${gauge_data['Gauge X']}`;
        gauge_div.appendChild(gauge_x_label);
        const gauge_x_input = document.createElement("input");
        gauge_x_input.id = "Window_Input";
        gauge_x_input.setAttribute("value", gauge_data['Gauge X'] || 0);
        gauge_x_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_x_input.value){
                    let value = gauge_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge X'] = value;
                    gauge_x_label.textContent = `Gauge X: ${value}`;
                    gauge_data['Gauge X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_x_input);
        const gauge_y_label = document.createElement('label');
        gauge_y_label.id = "Window_Label";
        gauge_y_label.textContent = `Gauge Y: ${gauge_data['Gauge Y']}`;
        gauge_div.appendChild(gauge_y_label);
        const gauge_y_input = document.createElement("input");
        gauge_y_input.id = "Window_Input";
        gauge_y_input.setAttribute("value", gauge_data['Gauge Y'] || 0);
        gauge_y_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_y_input.value){
                    let value = gauge_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge Y'] = value;
                    gauge_y_label.textContent = `Gauge Y: ${value}`;
                    gauge_data['Gauge Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_y_input);
        const gauge_w_label = document.createElement('label');
        gauge_w_label.id = "Window_Label";
        gauge_w_label.textContent = `Gauge Width: ${gauge_data['Gauge Width']}`;
        gauge_div.appendChild(gauge_w_label);
        const gauge_w_input = document.createElement("input");
        gauge_w_input.id = "Window_Input";
        gauge_w_input.setAttribute("value", gauge_data['Gauge Width'] || 0);
        gauge_w_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_w_input.value){
                    let value = gauge_w_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge Width'] = value;
                    gauge_w_label.textContent = `Gauge Width: ${value}`;
                    gauge_data['Gauge Width'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_w_input);
        const gauge_h_label = document.createElement('label');
        gauge_h_label.id = "Window_Label";
        gauge_h_label.textContent = `Gauge Height: ${gauge_data['Gauge Height']}`;
        gauge_div.appendChild(gauge_h_label);
        const gauge_h_input = document.createElement("input");
        gauge_h_input.id = "Window_Input";
        gauge_h_input.setAttribute("value", gauge_data['Gauge Height'] || 0);
        gauge_h_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_h_input.value){
                    let value = gauge_h_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge Height'] = value;
                    gauge_h_label.textContent = `Gauge Height: ${value}`;
                    gauge_data['Gauge Height'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_h_input);
        const gauge_b_label = document.createElement('label');
        gauge_b_label.id = "Window_Label";
        gauge_b_label.textContent = `Gauge Border: ${gauge_data['Gauge Border']}`;
        gauge_div.appendChild(gauge_b_label);
        const gauge_b_input = document.createElement("input");
        gauge_b_input.id = "Window_Input";
        gauge_b_input.setAttribute("value", gauge_data['Gauge Border'] || 0);
        gauge_b_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_b_input.value){
                    let value = gauge_b_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge Border'] = value;
                    gauge_b_label.textContent = `Gauge Border: ${value}`;
                    gauge_data['Gauge Border'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_b_input);
        const gauge_border_color_label = document.createElement('label');
        gauge_border_color_label.id = "Window_Label";
        gauge_border_color_label.textContent = `Gauge Border Color: ${gauge_data['Gauge Border Color']}`;
        gauge_div.appendChild(gauge_border_color_label);
        const gauge_border_color_input = document.createElement("input");
        gauge_border_color_input.id = "Window_Input";
        gauge_border_color_input.setAttribute("value", gauge_data['Gauge Border Color']);
        gauge_border_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_border_color_input.value){
                    let value = gauge_border_color_input.value;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge Border Color'] = value;
                    gauge_border_color_label.textContent = `Gauge Border Color: ${value}`;
                    gauge_data['Gauge Border Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_border_color_input);
        const gauge_background_color_label = document.createElement('label');
        gauge_background_color_label.id = "Window_Label";
        gauge_background_color_label.textContent = `Gauge Background Color: ${gauge_data['Gauge Background Color']}`;
        gauge_div.appendChild(gauge_background_color_label);
        const gauge_background_color_input = document.createElement("input");
        gauge_background_color_input.id = "Window_Input";
        gauge_background_color_input.setAttribute("value", gauge_data['Gauge Background Color']);
        gauge_background_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_background_color_input.value){
                    let value = gauge_background_color_input.value;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge Background Color'] = value;
                    gauge_background_color_label.textContent = `Gauge Background Color: ${value}`;
                    gauge_data['Gauge Background Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_background_color_input);
        const gauge_color_label = document.createElement('label');
        gauge_color_label.id = "Window_Label";
        gauge_color_label.textContent = `Gauge Color: ${gauge_data['Gauge Color']}`;
        gauge_div.appendChild(gauge_color_label);
        const gauge_color_input = document.createElement("input");
        gauge_color_input.id = "Window_Input";
        gauge_color_input.setAttribute("value", gauge_data['Gauge Color']);
        gauge_color_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = collapse_button._saved_index;
            if(sav_index >= 0){
                if(gauge_color_input.value){
                    let value = gauge_color_input.value;
                    menu['Basic Windows'][index]['Gauges'][sav_index]['Gauge Color'] = value;
                    gauge_color_label.textContent = `Gauge Color: ${value}`;
                    gauge_data['Gauge Color'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        gauge_div.appendChild(gauge_color_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const del_index = collapse_button._saved_index;
            if(del_index >= 0){
                menu['Basic Windows'][index]['Gauges'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        gauge_div.appendChild(delete_button);
        index_g++;
    })
    const add_gauge_button = document.createElement('button');
    add_gauge_button.id = "Add_Button";
    add_gauge_button.type = 'button';
    add_gauge_button.textContent = "ADD GAUGE";
    add_gauge_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const gauges = menu['Basic Windows'][index]['Gauges'];
        const last_gauge = gauges[gauges.length - 1];
        let new_index = 0;
        if(last_gauge){
            new_index = eval(last_gauge['ID']) + 1;
        }
        const gauge_obj = {
            "ID":`${new_index}`,
            "Label":"",
            "Label X":"0",
            "Label Y":"0",
            "Gauge Current Value":"0",
            "Gauge Max Value":"0",
            "Gauge X":"0",
            "Gauge Y":"0",
            "Gauge Width":"0",
            "Gauge Height":"0",
            "Gauge Border":"0",
            "Gauge Border Color":"#000000",
            "Gauge Background Color":"#666666",
            "Gauge Color":"#aaffaa"
        }
        gauges.push(gauge_obj);
        recompileMenus(menu_data, menu, true);
    })
    gauge_config_container.appendChild(add_gauge_button);
}

const addVarRequirements = function(requirements, req_div, data, option_data, type_req){
    const var_collapse_button = document.createElement('button');
    var_collapse_button.id = 'open_list_button';
    var_collapse_button.classList.add('collapsible');
    var_collapse_button.type = 'button';
    var_collapse_button.textContent = "Variable Requirements";
    var_collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Sub_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    req_div.appendChild(var_collapse_button);
    const variable_list_div = document.createElement('div');
    variable_list_div.id = `Button_Sub_Container`;
    variable_list_div.classList.add("content");
    req_div.appendChild(variable_list_div);
    const existing_variables = requirements['Variable Requirements'];
    existing_variables.forEach((var_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = var_data['Name'];
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Variable_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        variable_list_div.appendChild(collapse_button);
        const var_div = document.createElement('div');
        var_div.id = "Variable_Container";
        var_div.classList.add("content");
        variable_list_div.appendChild(var_div);
        const var_id_label = document.createElement('label');
        var_id_label.id = "Window_Label";
        var_id_label.textContent = `ID: ${var_data['Variable']}`;
        var_div.appendChild(var_id_label);
        const var_id_input = document.createElement("input");
        var_id_input.id = "Window_Input";
        var_id_input.setAttribute("value", var_data['Variable']);
        var_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Variable Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(var_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const variable_requirements = requirements['Variable Requirements'];
                let sav_index = -1;
                for(let i = 0; i < variable_requirements.length; i++){
                    const v_req = variable_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(var_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = var_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    variable_requirements[sav_index]['Variable'] = value;
                    var_data['Variable'] = value;
                    var_id_label.textContent = `ID: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        var_div.appendChild(var_id_input);
        const var_min_label = document.createElement('label');
        var_min_label.id = "Window_Label";
        var_min_label.textContent = `Min Value: ${var_data['Min Value']}`;
        var_div.appendChild(var_min_label);
        const var_min_input = document.createElement("input");
        var_min_input.id = "Window_Input";
        var_min_input.setAttribute("value", var_data['Min Value']);
        var_min_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Variable Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(var_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const variable_requirements = requirements['Variable Requirements'];
                let sav_index = -1;
                for(let i = 0; i < variable_requirements.length; i++){
                    const v_req = variable_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(var_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = var_min_input.value;
                    if(isNaN(eval(value)))value = 0;
                    variable_requirements[sav_index]['Min Value'] = value;
                    var_data['Min Value'] = value;
                    var_min_label.textContent = `Min Value: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        var_div.appendChild(var_min_input);
        const var_max_label = document.createElement('label');
        var_max_label.id = "Window_Label";
        var_max_label.textContent = `Max Value: ${var_data['Max Value']}`;
        var_div.appendChild(var_max_label);
        const var_max_input = document.createElement("input");
        var_max_input.id = "Window_Input";
        var_max_input.setAttribute("value", var_data['Max Value']);
        var_max_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Variable Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(var_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const variable_requirements = requirements['Variable Requirements'];
                let sav_index = -1;
                for(let i = 0; i < variable_requirements.length; i++){
                    const v_req = variable_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(var_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = var_max_input.value;
                    if(isNaN(eval(value)))value = 0;
                    variable_requirements[sav_index]['Max Value'] = value;
                    var_data['Max Value'] = value;
                    var_max_label.textContent = `Max Value: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        var_div.appendChild(var_max_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Variable Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(var_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const variable_requirements = requirements['Variable Requirements'];
                let del_index = -1;
                for(let i = 0; i < variable_requirements.length; i++){
                    const v_req = variable_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(var_data);
                    if(str_req == str_data){
                        del_index = i;
                        break;
                    }
                }
                if(del_index >= 0){
                    variable_requirements.splice(del_index, 1);
                    recompileMenus(menu_data, menu, true);
                }
            }
        })
        var_div.appendChild(delete_button);
    })
    const add_variable_button = document.createElement('button');
    add_variable_button.id = "Add_Button";
    add_variable_button.type = 'button';
    add_variable_button.textContent = "ADD VARIABLE";
    add_variable_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        let index = -1;
        for(let i = 0; i < options.length; i++){
            if(options[i] == option_data){
                index = i;
                break;
            }
        }
        if(index >= 0){
            const new_var = {
                "Name":"Variable",
                "Variable":"0",
                "Min Value":"0",
                "Max Value":"0"
            }
            const option = options[index];
            const display_req = option[`${type_req} Requirements`];
            display_req['Variable Requirements'].push(new_var);
            recompileMenus(menu_data, menu, true);
        }
    })
    variable_list_div.appendChild(add_variable_button);
}

const addSwRequirements = function(requirements, req_div, data, option_data, type_req){
    const sw_collapse_button = document.createElement('button');
    sw_collapse_button.id = 'open_list_button';
    sw_collapse_button.classList.add('collapsible');
    sw_collapse_button.type = 'button';
    sw_collapse_button.textContent = "Switch Requirements";
    sw_collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Sub_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    req_div.appendChild(sw_collapse_button);
    const switch_list_div = document.createElement('div');
    switch_list_div.id = `Button_Sub_Container`;
    switch_list_div.classList.add("content");
    req_div.appendChild(switch_list_div);
    const existing_switches = requirements['Switch Requirements'];
    existing_switches.forEach((sw_data)=>{
        const sw_div = document.createElement('div');
        sw_div.id = "Switch_Container";
        switch_list_div.appendChild(sw_div);
        const sw_id_label = document.createElement('label');
        sw_id_label.id = "Window_Label";
        sw_id_label.textContent = `ID: ${sw_data}`;
        sw_div.appendChild(sw_id_label);
        const sw_id_input = document.createElement("input");
        sw_id_input.id = "Window_Input";
        sw_id_input.setAttribute("value", sw_data);
        sw_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const sw_reqs = display_req['Switch Requirements'];
                return sw_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(sw_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const switch_requirements = requirements['Switch Requirements'];
                let sav_index = -1;
                for(let i = 0; i < switch_requirements.length; i++){
                    const s_req = switch_requirements[i];
                    const str_req = JSON.stringify(s_req);
                    const str_data = JSON.stringify(sw_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = sw_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    switch_requirements[sav_index] = value;
                    sw_data = value;
                    sw_id_label.textContent = `ID: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        sw_div.appendChild(sw_id_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const sw_reqs = display_req['Switch Requirements'];
                return sw_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(sw_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const switch_requirements = requirements['Switch Requirements'];
                let del_index = -1;
                for(let i = 0; i < switch_requirements.length; i++){
                    const s_req = switch_requirements[i];
                    const str_req = JSON.stringify(s_req);
                    const str_data = JSON.stringify(sw_data);
                    if(str_req == str_data){
                        del_index = i;
                        break;
                    }
                }
                if(del_index >= 0){
                    switch_requirements.splice(del_index, 1);
                    recompileMenus(menu_data, menu, true);
                }
            }
        })
        sw_div.appendChild(delete_button);
    })
    const add_switch_button = document.createElement('button');
    add_switch_button.id = "Add_Button";
    add_switch_button.type = 'button';
    add_switch_button.textContent = "ADD SWITCH";
    add_switch_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        let index = -1;
        for(let i = 0; i < options.length; i++){
            if(options[i] == option_data){
                index = i;
                break;
            }
        }
        if(index >= 0){
            const option = options[index];
            const display_req = option[`${type_req} Requirements`];
            display_req['Switch Requirements'].push("0");
            recompileMenus(menu_data, menu, true);
        }
    })
    switch_list_div.appendChild(add_switch_button);
}

const addItmRequirements = function(requirements, req_div, data, option_data, type_req){
    const item_collapse_button = document.createElement('button');
    item_collapse_button.id = 'open_list_button';
    item_collapse_button.classList.add('collapsible');
    item_collapse_button.type = 'button';
    item_collapse_button.textContent = "Item Requirements";
    item_collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Sub_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    req_div.appendChild(item_collapse_button);
    const item_list_div = document.createElement('div');
    item_list_div.id = `Button_Sub_Container`;
    item_list_div.classList.add("content");
    req_div.appendChild(item_list_div);
    const existing_items = requirements['Item Requirements'];
    existing_items.forEach((item_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = item_data['Name'];
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Item_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        item_list_div.appendChild(collapse_button);
        const item_div = document.createElement('div');
        item_div.id = "Item_Container";
        item_div.classList.add("content");
        item_list_div.appendChild(item_div);
        const itm_id_label = document.createElement('label');
        itm_id_label.id = "Window_Label";
        itm_id_label.textContent = `ID: ${item_data['Item']}`;
        item_div.appendChild(itm_id_label);
        const item_id_input = document.createElement("input");
        item_id_input.id = "Window_Input";
        item_id_input.setAttribute("value", item_data['Item']);
        item_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Item Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(item_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const item_requirements = requirements['Item Requirements'];
                let sav_index = -1;
                for(let i = 0; i < item_requirements.length; i++){
                    const i_req = item_requirements[i];
                    const str_req = JSON.stringify(i_req);
                    const str_data = JSON.stringify(item_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = item_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    item_requirements[sav_index]['Item'] = value;
                    item_data['Item'] = value;
                    itm_id_label.textContent = `ID: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        item_div.appendChild(item_id_input);
        const item_min_label = document.createElement('label');
        item_min_label.id = "Window_Label";
        item_min_label.textContent = `Min Value: ${item_data['Min Value']}`;
        item_div.appendChild(item_min_label);
        const item_min_input = document.createElement("input");
        item_min_input.id = "Window_Input";
        item_min_input.setAttribute("value", item_data['Min Value']);
        item_min_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Item Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(item_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const item_requirements = requirements['Item Requirements'];
                let sav_index = -1;
                for(let i = 0; i < item_requirements.length; i++){
                    const v_req = item_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(item_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = item_min_input.value;
                    if(isNaN(eval(value)))value = 0;
                    item_requirements[sav_index]['Min Value'] = value;
                    item_data['Min Value'] = value;
                    item_min_label.textContent = `Min Value: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        item_div.appendChild(item_min_input);
        const item_max_label = document.createElement('label');
        item_max_label.id = "Window_Label";
        item_max_label.textContent = `Max Value: ${item_data['Max Value']}`;
        item_div.appendChild(item_max_label);
        const item_max_input = document.createElement("input");
        item_max_input.id = "Window_Input";
        item_max_input.setAttribute("value", item_data['Max Value']);
        item_max_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Item Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(item_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const item_requirements = requirements['Item Requirements'];
                let sav_index = -1;
                for(let i = 0; i < item_requirements.length; i++){
                    const i_req = item_requirements[i];
                    const str_req = JSON.stringify(i_req);
                    const str_data = JSON.stringify(item_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = item_max_input.value;
                    if(isNaN(eval(value)))value = 0;
                    item_requirements[sav_index]['Max Value'] = value;
                    item_data['Max Value'] = value;
                    item_max_label.textContent = `Max Value: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        item_div.appendChild(item_max_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Item Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(item_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const item_requirements = requirements['Item Requirements'];
                let del_index = -1;
                for(let i = 0; i < item_requirements.length; i++){
                    const v_req = item_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(item_data);
                    if(str_req == str_data){
                        del_index = i;
                        break;
                    }
                }
                if(del_index >= 0){
                    item_requirements.splice(del_index, 1);
                    recompileMenus(menu_data, menu, true);
                }
            }
        })
        item_div.appendChild(delete_button);
    })
    const add_item_button = document.createElement('button');
    add_item_button.id = "Add_Button";
    add_item_button.type = 'button';
    add_item_button.textContent = "ADD ITEM";
    add_item_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        let index = -1;
        for(let i = 0; i < options.length; i++){
            if(options[i] == option_data){
                index = i;
                break;
            }
        }
        if(index >= 0){
            const new_var = {
                "Name":"Item",
                "Item":"0",
                "Min Value":"0",
                "Max Value":"0"
            }
            const option = options[index];
            const display_req = option[`${type_req} Requirements`];
            display_req['Item Requirements'].push(new_var);
            recompileMenus(menu_data, menu, true);
        }
    })
    item_list_div.appendChild(add_item_button);
}

const addWepRequirements = function(requirements, req_div, data, option_data, type_req){
    const wep_collapse_button = document.createElement('button');
    wep_collapse_button.id = 'open_list_button';
    wep_collapse_button.classList.add('collapsible');
    wep_collapse_button.type = 'button';
    wep_collapse_button.textContent = "Weapon Requirements";
    wep_collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Sub_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    req_div.appendChild(wep_collapse_button);
    const wep_list_div = document.createElement('div');
    wep_list_div.id = `Button_Sub_Container`;
    wep_list_div.classList.add("content");
    req_div.appendChild(wep_list_div);
    const existing_weapons = requirements['Weapon Requirements'];
    existing_weapons.forEach((wep_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = wep_data['Name'];
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Weapon_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        wep_list_div.appendChild(collapse_button);
        const wep_div = document.createElement('div');
        wep_div.id = "Weapon_Container";
        wep_div.classList.add("content");
        wep_list_div.appendChild(wep_div);
        const wep_id_label = document.createElement('label');
        wep_id_label.id = "Window_Label";
        wep_id_label.textContent = `ID: ${wep_data['Weapon']}`;
        wep_div.appendChild(wep_id_label);
        const wep_id_input = document.createElement("input");
        wep_id_input.id = "Window_Input";
        wep_id_input.setAttribute("value", wep_data['Weapon']);
        wep_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Weapon Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(wep_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const weapon_requirements = requirements['Weapon Requirements'];
                let sav_index = -1;
                for(let i = 0; i < weapon_requirements.length; i++){
                    const i_req = weapon_requirements[i];
                    const str_req = JSON.stringify(i_req);
                    const str_data = JSON.stringify(wep_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = wep_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    weapon_requirements[sav_index]['Weapon'] = value;
                    wep_data['Weapon'] = value;
                    wep_id_label.textContent = `ID: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        wep_div.appendChild(wep_id_input);
        const wep_min_label = document.createElement('label');
        wep_min_label.id = "Window_Label";
        wep_min_label.textContent = `Min Value: ${wep_data['Min Value']}`;
        wep_div.appendChild(wep_min_label);
        const wep_min_input = document.createElement("input");
        wep_min_input.id = "Window_Input";
        wep_min_input.setAttribute("value", wep_data['Min Value']);
        wep_min_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Weapon Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(wep_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const weapon_requirements = requirements['Weapon Requirements'];
                let sav_index = -1;
                for(let i = 0; i < weapon_requirements.length; i++){
                    const v_req = weapon_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(wep_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = wep_min_input.value;
                    if(isNaN(eval(value)))value = 0;
                    weapon_requirements[sav_index]['Min Value'] = value;
                    wep_data['Min Value'] = value;
                    wep_min_label.textContent = `Min Value: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        wep_div.appendChild(wep_min_input);
        const wep_max_label = document.createElement('label');
        wep_max_label.id = "Window_Label";
        wep_max_label.textContent = `Max Value: ${wep_data['Max Value']}`;
        wep_div.appendChild(wep_max_label);
        const wep_max_input = document.createElement("input");
        wep_max_input.id = "Window_Input";
        wep_max_input.setAttribute("value", wep_data['Max Value']);
        wep_max_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Weapon Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(wep_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const weapon_requirements = requirements['Weapon Requirements'];
                let sav_index = -1;
                for(let i = 0; i < weapon_requirements.length; i++){
                    const i_req = weapon_requirements[i];
                    const str_req = JSON.stringify(i_req);
                    const str_data = JSON.stringify(wep_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = wep_max_input.value;
                    if(isNaN(eval(value)))value = 0;
                    weapon_requirements[sav_index]['Max Value'] = value;
                    wep_data['Max Value'] = value;
                    wep_max_label.textContent = `Max Value: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        wep_div.appendChild(wep_max_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Weapon Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(wep_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const weapon_requirements = requirements['Weapon Requirements'];
                let del_index = -1;
                for(let i = 0; i < weapon_requirements.length; i++){
                    const v_req = weapon_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(wep_data);
                    if(str_req == str_data){
                        del_index = i;
                        break;
                    }
                }
                if(del_index >= 0){
                    weapon_requirements.splice(del_index, 1);
                    recompileMenus(menu_data, menu, true);
                }
            }
        })
        wep_div.appendChild(delete_button);
    })
    const add_weapon_button = document.createElement('button');
    add_weapon_button.id = "Add_Button";
    add_weapon_button.type = 'button';
    add_weapon_button.textContent = "ADD WEAPON";
    add_weapon_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        let index = -1;
        for(let i = 0; i < options.length; i++){
            if(options[i] == option_data){
                index = i;
                break;
            }
        }
        if(index >= 0){
            const new_var = {
                "Name":"Weapon",
                "Weapon":"0",
                "Min Value":"0",
                "Max Value":"0"
            }
            const option = options[index];
            const display_req = option[`${type_req} Requirements`];
            display_req['Weapon Requirements'].push(new_var);
            recompileMenus(menu_data, menu, true);
        }
    })
    wep_list_div.appendChild(add_weapon_button);
}

const addArmRequirements = function(requirements, req_div, data, option_data, type_req){
    const arm_collapse_button = document.createElement('button');
    arm_collapse_button.id = 'open_list_button';
    arm_collapse_button.classList.add('collapsible');
    arm_collapse_button.type = 'button';
    arm_collapse_button.textContent = "Armor Requirements";
    arm_collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Sub_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    req_div.appendChild(arm_collapse_button);
    const arm_list_div = document.createElement('div');
    arm_list_div.id = `Button_Sub_Container`;
    arm_list_div.classList.add("content");
    req_div.appendChild(arm_list_div);
    const existing_weapons = requirements['Armor Requirements'];
    existing_weapons.forEach((arm_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = arm_data['Name'];
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Armor_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        arm_list_div.appendChild(collapse_button);
        const arm_div = document.createElement('div');
        arm_div.id = "Armor_Container";
        arm_div.classList.add("content");
        arm_list_div.appendChild(arm_div);
        const arm_id_label = document.createElement('label');
        arm_id_label.id = "Window_Label";
        arm_id_label.textContent = `ID: ${arm_data['Armor']}`;
        arm_div.appendChild(arm_id_label);
        const arm_id_input = document.createElement("input");
        arm_id_input.id = "Window_Input";
        arm_id_input.setAttribute("value", arm_data['Armor']);
        arm_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const arm_reqs = display_req['Armor Requirements'];
                return arm_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(arm_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const armor_requirements = requirements['Armor Requirements'];
                let sav_index = -1;
                for(let i = 0; i < armor_requirements.length; i++){
                    const i_req = armor_requirements[i];
                    const str_req = JSON.stringify(i_req);
                    const str_data = JSON.stringify(arm_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = arm_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    armor_requirements[sav_index]['Armor'] = value;
                    arm_data['Armor'] = value;
                    arm_id_label.textContent = `ID: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        arm_div.appendChild(arm_id_input);
        const arm_min_label = document.createElement('label');
        arm_min_label.id = "Window_Label";
        arm_min_label.textContent = `Min Value: ${arm_data['Min Value']}`;
        arm_div.appendChild(arm_min_label);
        const arm_min_input = document.createElement("input");
        arm_min_input.id = "Window_Input";
        arm_min_input.setAttribute("value", arm_data['Min Value']);
        arm_min_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Armor Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(arm_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const armor_requirements = requirements['Armor Requirements'];
                let sav_index = -1;
                for(let i = 0; i < armor_requirements.length; i++){
                    const v_req = armor_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(arm_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = arm_min_input.value;
                    if(isNaN(eval(value)))value = 0;
                    armor_requirements[sav_index]['Min Value'] = value;
                    arm_data['Min Value'] = value;
                    arm_min_label.textContent = `Min Value: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        arm_div.appendChild(arm_min_input);
        const arm_max_label = document.createElement('label');
        arm_max_label.id = "Window_Label";
        arm_max_label.textContent = `Max Value: ${arm_data['Max Value']}`;
        arm_div.appendChild(arm_max_label);
        const arm_max_input = document.createElement("input");
        arm_max_input.id = "Window_Input";
        arm_max_input.setAttribute("value", arm_data['Max Value']);
        arm_max_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Armor Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(arm_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const armor_requirements = requirements['Armor Requirements'];
                let sav_index = -1;
                for(let i = 0; i < armor_requirements.length; i++){
                    const i_req = armor_requirements[i];
                    const str_req = JSON.stringify(i_req);
                    const str_data = JSON.stringify(arm_data);
                    if(str_req == str_data){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    let value = arm_max_input.value;
                    if(isNaN(eval(value)))value = 0;
                    armor_requirements[sav_index]['Max Value'] = value;
                    arm_data['Max Value'] = value;
                    arm_max_label.textContent = `Max Value: ${value}`;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        arm_div.appendChild(arm_max_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                const display_req = optn[`${type_req} Requirements`];
                const var_reqs = display_req['Armor Requirements'];
                return var_reqs.some((req)=>{
                    const str_req = JSON.stringify(req);
                    const str_data = JSON.stringify(arm_data);
                    return str_req == str_data;
                });
            })
            if(option){
                const requirements = option[`${type_req} Requirements`];
                const armor_requirements = requirements['Armor Requirements'];
                let del_index = -1;
                for(let i = 0; i < armor_requirements.length; i++){
                    const v_req = armor_requirements[i];
                    const str_req = JSON.stringify(v_req);
                    const str_data = JSON.stringify(arm_data);
                    if(str_req == str_data){
                        del_index = i;
                        break;
                    }
                }
                if(del_index >= 0){
                    armor_requirements.splice(del_index, 1);
                    recompileMenus(menu_data, menu, true);
                }
            }
        })
        arm_div.appendChild(delete_button);
    })
    const add_armor_button = document.createElement('button');
    add_armor_button.id = "Add_Button";
    add_armor_button.type = 'button';
    add_armor_button.textContent = "ADD ARMOR";
    add_armor_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        let index = -1;
        for(let i = 0; i < options.length; i++){
            if(options[i] == option_data){
                index = i;
                break;
            }
        }
        if(index >= 0){
            const new_var = {
                "Name":"Armor",
                "Armor":"0",
                "Min Value":"0",
                "Max Value":"0"
            }
            const option = options[index];
            const display_req = option[`${type_req} Requirements`];
            display_req['Armor Requirements'].push(new_var);
            recompileMenus(menu_data, menu, true);
        }
    })
    arm_list_div.appendChild(add_armor_button);
}

const addOptionDisplayRequirements = function(option_div, option_data, data){
    const requirements = option_data['Display Requirements'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Display Requirements";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    option_div.appendChild(collapse_button);
    const req_div = document.createElement('div');
    req_div.id = "Button_Container";
    req_div.classList.add("content");
    option_div.appendChild(req_div);
    addVarRequirements(requirements, req_div, data, option_data, 'Display');
    addSwRequirements(requirements, req_div, data, option_data, 'Display');
    addItmRequirements(requirements, req_div, data, option_data, 'Display');
    addWepRequirements(requirements, req_div, data, option_data, 'Display');
    addArmRequirements(requirements, req_div, data, option_data, 'Display');
}

const addOptionSelectRequirements = function(option_div, option_data, data){
    const requirements = option_data['Select Requirements'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Select Requirements";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    option_div.appendChild(collapse_button);
    const req_div = document.createElement('div');
    req_div.id = "Button_Container";
    req_div.classList.add("content");
    option_div.appendChild(req_div);
    addVarRequirements(requirements, req_div, data, option_data, 'Select');
    addSwRequirements(requirements, req_div, data, option_data, 'Select');
    addItmRequirements(requirements, req_div, data, option_data, 'Select');
    addWepRequirements(requirements, req_div, data, option_data, 'Select');
    addArmRequirements(requirements, req_div, data, option_data, 'Select');
}

const addWindowOptionPictures = function(option_div, option_data, data){
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Pictures";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    option_div.appendChild(collapse_button);
    const picture_container_div = document.createElement('div');
    picture_container_div.id = "Button_Container";
    option_div.appendChild(picture_container_div);
    const existing_pictures = option_data['Pictures'] || [];
    existing_pictures.forEach((picture)=>{
        const pic_label = document.createElement('label');
        pic_label.id = "Window_Label";
        pic_label.textContent = `File: ${picture}`;
        picture_container_div.appendChild(pic_label);
        const picture_input = document.createElement('input');
        picture_input.id = "Window_Input";
        picture_input.setAttribute("type", "file");
        picture_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = picture_input._saved_index;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                return optn == option_data;
            })
            if(option){
                const pictures = option['Pictures'];
                for(let i = 0; i < pictures.length; i++){
                    if(pictures[i] == picture){
                        sav_index = i;
                        break;
                    }
                }
                if(sav_index >= 0){
                    if(picture_input.value){
                        const input_paths = (picture_input.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
                        if(input_paths.length > 0){
                            const path = input_paths[0];
                            let file_name = path.replace("img\\pictures\\", "");
                            file_name = file_name.replace(".png", "");
                            file_name = file_name.replace("\\", "/");
                            pictures[sav_index] = file_name;
                            pic_label.textContent = `File: ${file_name}`;
                            existing_pictures[sav_index] = file_name;
                        }
                        recompileMenus(menu_data, menu);
                    }
                }
            }
        })
        picture_container_div.appendChild(picture_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let del_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            const option = options.find((optn)=>{
                return optn == option_data;
            })
            if(option){
                const pictures = option['Pictures'];
                for(let i = 0; i < pictures.length; i++){
                    if(pictures[i] == picture){
                        del_index = i;
                        break;
                    }
                }
                if(del_index >= 0){
                    pictures.splice(del_index, 1);
                    recompileMenus(menu_data, menu, true);
                }
            }
        })
        picture_container_div.appendChild(delete_button);
    })
    const add_picture_button = document.createElement('button');
    add_picture_button.id = "Add_Button";
    add_picture_button.type = 'button';
    add_picture_button.textContent = "ADD PICTURE";
    add_picture_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn == option_data;
        })
        if(option){
            option['Pictures'].push("");
            recompileMenus(menu_data, menu, true);
        }
    })
    picture_container_div.appendChild(add_picture_button);
}

const addWindowOptionStaticGraphic = function(option_div, option_data, data){
    const static_gfx_data = option_data['Static Graphic'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Static Graphic";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    option_div.appendChild(collapse_button);
    const gfx_container_div = document.createElement('div');
    gfx_container_div.id = "Button_Container";
    option_div.appendChild(gfx_container_div);
    const file_label = document.createElement('label');
    file_label.id = "Window_Label";
    file_label.textContent = `File: ${static_gfx_data['File'] || ""}`;
    gfx_container_div.appendChild(file_label);
    const file_input = document.createElement('input');
    file_input.id = "Window_Input";
    file_input.setAttribute("type", "file");
    file_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            const input_paths = (file_input.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
            if(input_paths.length > 0){
                const path = input_paths[0];
                let file_name = path.replace("img\\pictures\\", "");
                file_name = file_name.replace(".png", "");
                file_name = file_name.replace("\\", "/");
                file_label.textContent = `File: ${file_name}`;
                static_gfx_data['File'] = file_name;
                option['Static Graphic']['File'] = file_name;
            }
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(file_input);
    const x_label = document.createElement('label');
    x_label.id = "Window_Label";
    x_label.textContent = `X: ${static_gfx_data['X']}`;
    gfx_container_div.appendChild(x_label);
    const x_input = document.createElement("input");
    x_input.id = "Window_Input";
    x_input.setAttribute("type", "text");
    x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = x_input.value;
            if(isNaN(eval(value)))value = 0;
            x_label.textContent = `X: ${value}`;
            static_gfx_data['X'] = value;
            option['Static Graphic']['X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(x_input);
    const y_label = document.createElement('label');
    y_label.id = "Window_Label";
    y_label.textContent = `Y: ${static_gfx_data['Y']}`;
    gfx_container_div.appendChild(y_label);
    const y_input = document.createElement("input");
    y_input.id = "Window_Input";
    y_input.setAttribute("type", "text");
    y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = y_input.value;
            if(isNaN(eval(value)))value = 0;
            y_label.textContent = `Y: ${value}`;
            static_gfx_data['Y'] = value;
            option['Static Graphic']['Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(y_input);
    const sx_label = document.createElement('label');
    sx_label.id = "Window_Label";
    sx_label.textContent = `Scrolling X: ${static_gfx_data['Scrolling X']}`;
    gfx_container_div.appendChild(sx_label);
    const sx_input = document.createElement("input");
    sx_input.id = "Window_Input";
    sx_input.setAttribute("type", "text");
    sx_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = sx_input.value;
            if(isNaN(eval(value)))value = 0;
            sx_label.textContent = `Scrolling X: ${value}`;
            static_gfx_data['Scrolling X'] = value;
            option['Static Graphic']['Scrolling X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(sx_input);
    const sy_label = document.createElement('label');
    sy_label.id = "Window_Label";
    sy_label.textContent = `Scrolling Y: ${static_gfx_data['Scrolling Y']}`;
    gfx_container_div.appendChild(sy_label);
    const sy_input = document.createElement("input");
    sy_input.id = "Window_Input";
    sy_input.setAttribute("type", "text");
    sy_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = sy_input.value;
            if(isNaN(eval(value)))value = 0;
            sy_label.textContent = `Scrolling Y: ${value}`;
            static_gfx_data['Scrolling Y'] = value;
            option['Static Graphic']['Scrolling Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(sy_input);
    const ax_label = document.createElement('label');
    ax_label.id = "Window_Label";
    ax_label.textContent = `Anchor X: ${static_gfx_data['Anchor X']}`;
    gfx_container_div.appendChild(ax_label);
    const ax_input = document.createElement("input");
    ax_input.id = "Window_Input";
    ax_input.setAttribute("type", "text");
    ax_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = ax_input.value;
            if(isNaN(eval(value)))value = 0;
            ax_label.textContent = `Anchor X: ${value}`;
            static_gfx_data['Anchor X'] = value;
            option['Static Graphic']['Anchor X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(ax_input);
    const ay_label = document.createElement('label');
    ay_label.id = "Window_Label";
    ay_label.textContent = `Anchor Y: ${static_gfx_data['Anchor Y']}`;
    gfx_container_div.appendChild(ay_label);
    const ay_input = document.createElement("input");
    ay_input.id = "Window_Input";
    ay_input.setAttribute("type", "text");
    ay_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = ay_input.value;
            if(isNaN(eval(value)))value = 0;
            ay_label.textContent = `Anchor Y: ${value}`;
            static_gfx_data['Anchor Y'] = value;
            option['Static Graphic']['Anchor Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(ay_input);
    const rot_label = document.createElement('label');
    rot_label.id = "Window_Label";
    rot_label.textContent = `Rotation: ${static_gfx_data['Rotation']} (${eval(static_gfx_data['Constant Rotation']) ? "constant" : "static"})`;
    gfx_container_div.appendChild(rot_label);
    const rot_input = document.createElement("input");
    rot_input.id = "Window_Input";
    rot_input.setAttribute("type", "text");
    rot_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = rot_input.value;
            if(isNaN(eval(value)))value = 0;
            rot_label.textContent = `Rotation: ${value}  (${eval(static_gfx_data['Constant Rotation']) ? "constant" : "static"})`;
            static_gfx_data['Rotation'] = value;
            option['Static Graphic']['Rotation'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(rot_input);
    const rot_constant = document.createElement("input");
    rot_constant.id = 'BG_Check';
    rot_constant.setAttribute("type", "checkbox");
    rot_constant.addEventListener("input", (event)=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            const value = rot_constant.checked;
            rot_label.textContent = `Rotation: ${static_gfx_data['Rotation']}  (${value ? "constant" : "static"})`;
            static_gfx_data['Constant Rotation'] = value;
            option['Static Graphic']['Constant Rotation'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(rot_constant);
}

const addWindowOptionAnimGraphic = function(option_div, option_data, data){
    const anim_gfx_data = option_data['Animated Graphic'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Animated Graphic";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    option_div.appendChild(collapse_button);
    const gfx_container_div = document.createElement('div');
    gfx_container_div.id = "Button_Container";
    option_div.appendChild(gfx_container_div);
    const file_label = document.createElement('label');
    file_label.id = "Window_Label";
    file_label.textContent = `File: ${anim_gfx_data['File'] || ""}`;
    gfx_container_div.appendChild(file_label);
    const file_input = document.createElement('input');
    file_input.id = "Window_Input";
    file_input.setAttribute("type", "file");
    file_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            const input_paths = (file_input.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
            if(input_paths.length > 0){
                const path = input_paths[0];
                let file_name = path.replace("img\\pictures\\", "");
                file_name = file_name.replace(".png", "");
                file_name = file_name.replace("\\", "/");
                file_label.textContent = `File: ${file_name}`;
                anim_gfx_data['File'] = file_name;
                option['Animated Graphic']['File'] = file_name;
            }
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(file_input);
    const x_label = document.createElement('label');
    x_label.id = "Window_Label";
    x_label.textContent = `X: ${anim_gfx_data['X']}`;
    gfx_container_div.appendChild(x_label);
    const x_input = document.createElement("input");
    x_input.id = "Window_Input";
    x_input.setAttribute("type", "text");
    x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = x_input.value;
            if(isNaN(eval(value)))value = 0;
            x_label.textContent = `X: ${value}`;
            anim_gfx_data['X'] = value;
            option['Animated Graphic']['X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(x_input);
    const y_label = document.createElement('label');
    y_label.id = "Window_Label";
    y_label.textContent = `Y: ${anim_gfx_data['Y']}`;
    gfx_container_div.appendChild(y_label);
    const y_input = document.createElement("input");
    y_input.id = "Window_Input";
    y_input.setAttribute("type", "text");
    y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = y_input.value;
            if(isNaN(eval(value)))value = 0;
            y_label.textContent = `Y: ${value}`;
            anim_gfx_data['Y'] = value;
            option['Animated Graphic']['Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(y_input);
    const frames_label = document.createElement('label');
    frames_label.id = "Window_Label";
    frames_label.textContent = `Max Frames: ${anim_gfx_data['Max Frames']}`;
    gfx_container_div.appendChild(frames_label);
    const frames_input = document.createElement("input");
    frames_input.id = "Window_Input";
    frames_input.setAttribute("type", "text");
    frames_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = frames_input.value;
            if(isNaN(eval(value)))value = 0;
            frames_label.textContent = `Max Frames: ${value}`;
            anim_gfx_data['Max Frames'] = value;
            option['Animated Graphic']['Max Frames'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(frames_input);
    const rate_label = document.createElement('label');
    rate_label.id = "Window_Label";
    rate_label.textContent = `Frame Rate: ${anim_gfx_data['Frame Rate']}`;
    gfx_container_div.appendChild(rate_label);
    const rate_input = document.createElement("input");
    rate_input.id = "Window_Input";
    rate_input.setAttribute("type", "text");
    rate_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = rate_input.value;
            if(isNaN(eval(value)))value = 0;
            rate_label.textContent = `Frame Rate: ${value}`;
            anim_gfx_data['Frame Rate'] = value;
            option['Animated Graphic']['Frame Rate'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    gfx_container_div.appendChild(rate_input);
}

const addWindowOptionSceneBtn = function(option_div, option_data, data){
    const btn_data = option_data['Scene Button'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Scene Button";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    option_div.appendChild(collapse_button);
    const btn_container_div = document.createElement('div');
    btn_container_div.id = "Button_Container";
    option_div.appendChild(btn_container_div);
    const name_label = document.createElement('label');
    name_label.id = "Window_Label";
    name_label.textContent = `Name: ${btn_data['Name'] || ""}`;
    btn_container_div.appendChild(name_label);
    const name_input = document.createElement('input');
    name_input.id = "Window_Input";
    name_input.setAttribute("type", "text");
    name_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = name_input.value;
            if(isNaN(eval(value)))value = 0;
            name_label.textContent = `Name: ${value}`;
            anim_gfx_data['Name'] = value;
            option['Animated Graphic']['Frame Rate'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    btn_container_div.appendChild(name_input);
    const cold_collapse_button = document.createElement('button');
    cold_collapse_button.id = 'open_list_button';
    cold_collapse_button.classList.add('collapsible');
    cold_collapse_button.type = 'button';
    cold_collapse_button.textContent = "Cold Graphic";
    cold_collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Window_Scene_Button_Gfx_Sub_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    btn_container_div.appendChild(cold_collapse_button);
    const cold_graphic = btn_data['Cold Graphic'];
    const cold_container_div = document.createElement('div');
    cold_container_div.id = "Window_Scene_Button_Sub_Container";
    btn_container_div.appendChild(cold_container_div);
    const cold_file_label = document.createElement('label');
    cold_file_label.id = "Window_Label";
    cold_file_label.textContent = `File: ${cold_graphic['File'] || ""}`;
    cold_container_div.appendChild(cold_file_label);
    const cold_file_input = document.createElement('input');
    cold_file_input.id = "Window_Input";
    cold_file_input.setAttribute("type", "file");
    cold_file_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            const input_paths = (cold_file_input.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
            if(input_paths.length > 0){
                const path = input_paths[0];
                let file_name = path.replace("img\\pictures\\", "");
                file_name = file_name.replace(".png", "");
                file_name = file_name.replace("\\", "/");
                cold_file_label.textContent = `File: ${file_name}`;
                cold_graphic['File'] = file_name;
                option['Scene Button']['Cold Graphic']['File'] = file_name;
            }
            recompileMenus(menu_data, menu);
        }
    })
    cold_container_div.appendChild(cold_file_input);
    const cold_x_label = document.createElement('label');
    cold_x_label.id = "Window_Label";
    cold_x_label.textContent = `X: ${cold_graphic['X']}`;
    cold_container_div.appendChild(cold_x_label);
    const cold_x_input = document.createElement("input");
    cold_x_input.id = "Window_Input";
    cold_x_input.setAttribute("type", "text");
    cold_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = cold_x_input.value;
            if(isNaN(eval(value)))value = 0;
            cold_x_label.textContent = `X: ${value}`;
            cold_graphic['X'] = value;
            option['Scene Button']['Cold Graphic']['X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    cold_container_div.appendChild(cold_x_input);
    const cold_y_label = document.createElement('label');
    cold_y_label.id = "Window_Label";
    cold_y_label.textContent = `Y: ${cold_graphic['Y']}`;
    cold_container_div.appendChild(cold_y_label);
    const cold_y_input = document.createElement("input");
    cold_y_input.id = "Window_Input";
    cold_y_input.setAttribute("type", "text");
    cold_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = cold_y_input.value;
            if(isNaN(eval(value)))value = 0;
            cold_y_label.textContent = `Y: ${value}`;
            cold_graphic['Y'] = value;
            option['Scene Button']['Cold Graphic']['Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    cold_container_div.appendChild(cold_y_input);
    const cold_frames_label = document.createElement('label');
    cold_frames_label.id = "Window_Label";
    cold_frames_label.textContent = `Max Frames: ${cold_graphic['Max Frames']}`;
    cold_container_div.appendChild(cold_frames_label);
    const cold_frames_input = document.createElement("input");
    cold_frames_input.id = "Window_Input";
    cold_frames_input.setAttribute("type", "text");
    cold_frames_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = cold_frames_input.value;
            if(isNaN(eval(value)))value = 0;
            cold_frames_label.textContent = `Max Frames: ${value}`;
            cold_graphic['Max Frames'] = value;
            option['Scene Button']['Cold Graphic']['Max Frames'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    cold_container_div.appendChild(cold_frames_input);
    const cold_rate_label = document.createElement('label');
    cold_rate_label.id = "Window_Label";
    cold_rate_label.textContent = `Frame Rate: ${cold_graphic['Frame Rate']}`;
    cold_container_div.appendChild(cold_rate_label);
    const cold_rate_input = document.createElement("input");
    cold_rate_input.id = "Window_Input";
    cold_rate_input.setAttribute("type", "text");
    cold_rate_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = cold_rate_input.value;
            if(isNaN(eval(value)))value = 0;
            cold_rate_label.textContent = `Frame Rate: ${value}`;
            cold_graphic['Frame Rate'] = value;
            option['Scene Button']['Cold Graphic']['Frame Rate'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    cold_container_div.appendChild(cold_rate_input);
    const hot_collapse_button = document.createElement('button');
    hot_collapse_button.id = 'open_list_button';
    hot_collapse_button.classList.add('collapsible');
    hot_collapse_button.type = 'button';
    hot_collapse_button.textContent = "Hot Graphic";
    hot_collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Window_Scene_Button_Gfx_Sub_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    btn_container_div.appendChild(hot_collapse_button);
    const hot_graphic = btn_data['Hot Graphic'];
    const hot_container_div = document.createElement('div');
    hot_container_div.id = "Window_Scene_Button_Sub_Container";
    btn_container_div.appendChild(hot_container_div);
    const hot_file_label = document.createElement('label');
    hot_file_label.id = "Window_Label";
    hot_file_label.textContent = `File: ${hot_graphic['File'] || ""}`;
    hot_container_div.appendChild(hot_file_label);
    const hot_file_input = document.createElement('input');
    hot_file_input.id = "Window_Input";
    hot_file_input.setAttribute("type", "file");
    hot_file_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            const input_paths = (hot_file_input.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
            if(input_paths.length > 0){
                const path = input_paths[0];
                let file_name = path.replace("img\\pictures\\", "");
                file_name = file_name.replace(".png", "");
                file_name = file_name.replace("\\", "/");
                hot_file_label.textContent = `File: ${file_name}`;
                hot_graphic['File'] = file_name;
                option['Scene Button']['Hot Graphic']['File'] = file_name;
            }
            recompileMenus(menu_data, menu);
        }
    })
    hot_container_div.appendChild(hot_file_input);
    const hot_x_label = document.createElement('label');
    hot_x_label.id = "Window_Label";
    hot_x_label.textContent = `X: ${hot_graphic['X']}`;
    hot_container_div.appendChild(hot_x_label);
    const hot_x_input = document.createElement("input");
    hot_x_input.id = "Window_Input";
    hot_x_input.setAttribute("type", "text");
    hot_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = hot_x_input.value;
            if(isNaN(eval(value)))value = 0;
            hot_x_label.textContent = `X: ${value}`;
            hot_graphic['X'] = value;
            option['Scene Button']['Hot Graphic']['X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    hot_container_div.appendChild(hot_x_input);
    const hot_y_label = document.createElement('label');
    hot_y_label.id = "Window_Label";
    hot_y_label.textContent = `Y: ${hot_graphic['Y']}`;
    hot_container_div.appendChild(hot_y_label);
    const hot_y_input = document.createElement("input");
    hot_y_input.id = "Window_Input";
    hot_y_input.setAttribute("type", "text");
    hot_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = hot_y_input.value;
            if(isNaN(eval(value)))value = 0;
            hot_y_label.textContent = `Y: ${value}`;
            hot_graphic['Y'] = value;
            option['Scene Button']['Hot Graphic']['Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    hot_container_div.appendChild(hot_y_input);
    const hot_frames_label = document.createElement('label');
    hot_frames_label.id = "Window_Label";
    hot_frames_label.textContent = `Max Frames: ${hot_graphic['Max Frames']}`;
    hot_container_div.appendChild(hot_frames_label);
    const hot_frames_input = document.createElement("input");
    hot_frames_input.id = "Window_Input";
    hot_frames_input.setAttribute("type", "text");
    hot_frames_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = hot_frames_input.value;
            if(isNaN(eval(value)))value = 0;
            hot_frames_label.textContent = `Max Frames: ${value}`;
            hot_graphic['Max Frames'] = value;
            option['Scene Button']['Hot Graphic']['Max Frames'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    hot_container_div.appendChild(hot_frames_input);
    const hot_rate_label = document.createElement('label');
    hot_rate_label.id = "Window_Label";
    hot_rate_label.textContent = `Frame Rate: ${hot_graphic['Frame Rate']}`;
    hot_container_div.appendChild(hot_rate_label);
    const hot_rate_input = document.createElement("input");
    hot_rate_input.id = "Window_Input";
    hot_rate_input.setAttribute("type", "text");
    hot_rate_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const option = options.find((optn)=>{
            return optn['Name'] == option_data['Name'];
        })
        if(option){
            let value = hot_rate_input.value;
            if(isNaN(eval(value)))value = 0;
            hot_rate_label.textContent = `Frame Rate: ${value}`;
            hot_graphic['Frame Rate'] = value;
            option['Scene Button']['Hot Graphic']['Frame Rate'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    hot_container_div.appendChild(hot_rate_input);
}

const addWindowSelcOptnForm = function(container, data){
    const selection_window = data['Selection Window'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Selection Options";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const option_config_container = document.createElement('div');
    option_config_container.id = "Button_Container";
    option_config_container.classList.add("content");
    container.appendChild(option_config_container);
    const existing_options = selection_window["Selection Options"];
    let optn_index = 0;
    existing_options.forEach((option_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${option_data['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Option_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        option_config_container.appendChild(collapse_button);
        const option_div = document.createElement('div');
        option_div.id = "Option_Container";
        option_div.classList.add("content");
        option_config_container.appendChild(option_div);
        const option_name_label = document.createElement('label');
        option_name_label.id = "Window_Label";
        option_name_label.textContent = `Name: ${option_data['Name']}`;
        option_div.appendChild(option_name_label);
        const option_name_input = document.createElement('input');
        option_name_input.setAttribute("value", option_data['Name']);
        option_name_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                const value = option_name_input.value;
                options[sav_index]['Name'] = value;
                option_data['Name'] = value;
                option_name_label.textContent = `Name: ${value}`;
                collapse_button.textContent = value;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_name_input);
        const option_alt_name_label = document.createElement('label');
        option_alt_name_label.id = "Window_Label";
        option_alt_name_label.textContent = `Alternative Name: ${option_data['Alternative Name']}`;
        option_div.appendChild(option_alt_name_label);
        const option_alt_name_input = document.createElement('input');
        option_alt_name_input.setAttribute("value", option_data['Alternative Name']);
        option_alt_name_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                const value = option_alt_name_input.value;
                options[sav_index]['Alternative Name'] = value;
                option_data['Alternative Name'] = value;
                option_alt_name_label.textContent = `Alternative Name: ${value}`;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_alt_name_input);
        addOptionDisplayRequirements(option_div, option_data, data);
        addOptionSelectRequirements(option_div, option_data, data);
        const option_desc_label = document.createElement('label');
        option_desc_label.id = "Window_Label";
        option_desc_label.textContent = `Description: ${option_data['Description']}`;
        option_div.appendChild(option_desc_label);
        const option_desc_input = document.createElement('input');
        option_desc_input.id = "Window_Input";
        option_desc_input.setAttribute("value", option_data['Description']);
        option_desc_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                const value = option_desc_input.value;
                options[sav_index]['Description'] = value;
                option_data['Description'] = value;
                option_desc_label.textContent = `Description: ${value}`;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_desc_input);
        addWindowOptionPictures(option_div, option_data, data);
        addWindowOptionStaticGraphic(option_div, option_data, data);
        addWindowOptionAnimGraphic(option_div, option_data, data);
        const option_video_label = document.createElement('label');
        option_video_label.id = "Window_Label";
        option_video_label.textContent = `Video: ${option_data['Video']}`;
        option_div.appendChild(option_video_label);
        const option_video_input = document.createElement("input");
        option_video_input.setAttribute("type", "file");
        option_video_input.setAttribute("accept", "video/webm");
        option_video_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                if(option_video_input.value){
                    const input_paths = (option_video_input.value || "").match(/(movies\\[a-z\d*|\W\D*]+)/gm);
                    if(input_paths.length > 0){
                        const path = input_paths[0];
                        let file_name = path.replace("movies\\", "");
                        file_name = file_name.replace(".webm", "");
                        file_name = file_name.replace("\\", "/");
                        options[sav_index]['Video'] = file_name;
                        option_video_label.textContent = `Video: ${file_name}`;
                        option_data['Video'] = file_name;
                    }
                    recompileMenus(menu_data, menu);
                }
            }
        })
        option_div.appendChild(option_video_input);
        const option_video_x_label = document.createElement('label');
        option_video_x_label.id = "Window_Label";
        option_video_x_label.textContent = `Video X: ${option_data['Video X']}`;
        option_div.appendChild(option_video_x_label);
        const option_video_x_input = document.createElement("input");
        option_video_x_input.id = "Window_Input";
        option_video_x_input.setAttribute("type", "text");
        option_video_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                let value = option_video_x_input.value;
                if(isNaN(eval(value)))value = 0;
                options[sav_index]['Video X'] = value;
                option_video_x_label.textContent = `Video X: ${value}`;
                option_data['Video X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_video_x_input);
        const option_video_y_label = document.createElement('label');
        option_video_y_label.id = "Window_Label";
        option_video_y_label.textContent = `Video Y: ${option_data['Video Y']}`;
        option_div.appendChild(option_video_y_label);
        const option_video_y_input = document.createElement("input");
        option_video_y_input.id = "Window_Input";
        option_video_y_input.setAttribute("type", "text");
        option_video_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                let value = option_video_y_input.value;
                if(isNaN(eval(value)))value = 0;
                options[sav_index]['Video Y'] = value;
                option_video_y_label.textContent = `Video Y: ${value}`;
                option_data['Video Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_video_y_input);
        const option_video_w_label = document.createElement('label');
        option_video_w_label.id = "Window_Label";
        option_video_w_label.textContent = `Video Width: ${option_data['Video Width']}`;
        option_div.appendChild(option_video_w_label);
        const option_video_w_input = document.createElement("input");
        option_video_w_input.id = "Window_Input";
        option_video_w_input.setAttribute("type", "text");
        option_video_w_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                let value = option_video_w_input.value;
                if(isNaN(eval(value)))value = 0;
                options[sav_index]['Video Width'] = value;
                option_video_w_label.textContent = `Video Width: ${value}`;
                option_data['Video Width'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_video_w_input);
        const option_video_h_label = document.createElement('label');
        option_video_h_label.id = "Window_Label";
        option_video_h_label.textContent = `Video Height: ${option_data['Video Height']}`;
        option_div.appendChild(option_video_h_label);
        const option_video_h_input = document.createElement("input");
        option_video_h_input.id = "Window_Input";
        option_video_h_input.setAttribute("type", "text");
        option_video_h_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                let value = option_video_h_input.value;
                if(isNaN(eval(value)))value = 0;
                options[sav_index]['Video Height'] = value;
                option_video_h_label.textContent = `Video Height: ${value}`;
                option_data['Video Height'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_video_h_input);
        addWindowOptionSceneBtn(option_div, option_data, data);
        const option_event_label = document.createElement('label');
        option_event_label.id = "Window_Label";
        option_event_label.textContent = `Event Execution: ${option_data['Event Execution']}`;
        option_div.appendChild(option_event_label);
        const option_event_input = document.createElement("input");
        option_event_input.id = "Window_Input";
        option_event_input.setAttribute("type", "text");
        option_event_input.setAttribute("value", option_data['Event Execution']);
        option_event_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                let value = option_event_input.value;
                if(isNaN(eval(value)))value = 0;
                options[sav_index]['Event Execution'] = value;
                option_event_label.textContent = `Event Execution: ${value}`;
                option_data['Event Execution'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_event_input);
        const option_code_label = document.createElement('label');
        option_code_label.id = "Window_Label";
        option_code_label.textContent = `Code`;
        option_div.appendChild(option_code_label);
        const option_code_input = document.createElement("input");
        option_code_input.id = "Window_Input";
        option_code_input.setAttribute("type", "text");
        option_code_input.setAttribute("value", option_data['Code Execution']);
        option_code_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                let value = option_code_input.value;
                options[sav_index]['Code Execution'] = value;
                option_code_label.textContent = `Code`;
                option_data['Code Execution'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_code_input);
        const option_actor_label = document.createElement('label');
        option_actor_label.id = "Window_Label";
        option_actor_label.textContent = `Require Actor Select: ${option_data['Require Actor Select'] ? "true" : "false"}`;
        option_div.appendChild(option_actor_label);
        const option_actor_input = document.createElement("input");
        option_actor_input.id = "Window_Input";
        option_actor_input.setAttribute("type", "checkbox");
        option_actor_input.checked = eval(option_data['Require Actor Select'])
        option_actor_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let sav_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    sav_index = i;
                    break;
                }
            }
            if(sav_index >= 0){
                let value = option_actor_input.checked;
                options[sav_index]['Require Actor Select'] = value;
                option_actor_label.textContent = `Require Actor Select: ${value ? "true" : "false"}`;
                option_data['Require Actor Select'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        option_div.appendChild(option_actor_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            let del_index = -1;
            const selection_window = menu['Selection Window'];
            const options = selection_window['Selection Options'];
            for(let i = 0; i < options.length; i++){
                const optn = options[i];
                if(optn['ID'] == option_data['ID']){
                    del_index = i;
                    break;
                }
            }
            if(del_index >= 0){
                selection_window['Selection Options'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        option_div.appendChild(delete_button);
    })
    const add_optn_button = document.createElement('button');
    add_optn_button.id = "Add_Button";
    add_optn_button.type = 'button';
    add_optn_button.textContent = "ADD OPTION";
    add_optn_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        const options = selection_window['Selection Options'];
        const new_index = options.length;
        const selc_obj = {
            "Name":`Option: ${new_index}`,
            "Alternative Name":"",
            "Display Requirements":{
                "Variable Requirements":[],
                "Switch Requirements":[],
                "Item Requirements":[],
                "Weapon Requirements":[],
                "Armor Requirements":[],
            },
            "Select Requirements":{
                "Variable Requirements":[],
                "Switch Requirements":[],
                "Item Requirements":[],
                "Weapon Requirements":[],
                "Armor Requirements":[],
            },
            "Description":"0",
            "Pictures":[],
            "Static Graphic":"0",
            "Animated Graphic":"0",
            "Video":"",
            "Video X":"0",
            "Video Y":"0",
            "Video Width":"0",
            "Video Height":"0",
            "Scene Button":"",
            "Event Execution":"0",
            "Code Execution":"console.log(`CHANGE ME!!!`)",
            "Require Actor Select":"false"
        }
        options.push(selc_obj);
        recompileMenus(menu_data, menu, true);
        optn_index++;
    })
    option_config_container.appendChild(add_optn_button);
}

const createUpdateCodeForm = function(form, data){
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Update Codes";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Button_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const update_code_container = document.createElement('div');
    update_code_container.id = "Button_Container";
    update_code_container.classList.add("content");
    form.appendChild(update_code_container);
    let index = 0;
    const codes = data['Update Codes'] || [];
    codes.forEach((code)=>{
        const code_div = document.createElement('div');
        code_div.id = "Code_Container"
        code_div._index = JSON.parse(JSON.stringify(index));
        update_code_container.appendChild(code_div);
        const code_label = document.createElement("label");
        code_label.id = "Window_Label";
        code_label.textContent = `Code: ${index}`;
        code_div.appendChild(code_label);
        const code_input = document.createElement("input");
        code_input.id = "Window_Input";
        code_input.setAttribute("value", code);
        code_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const codes = menu['Update Codes'];
            const new_code = code_input.value;
            codes[code_div._index] = new_code;
            data['Update Codes'][code_div._index] = new_code;
            recompileMenus(menu_data, menu);
        })
        code_div.appendChild(code_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const del_index = collapse_button._saved_index;
            if(del_index >= 0){
                menu['Update Codes'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        code_div.appendChild(delete_button);
    })
    const add_gauge_button = document.createElement('button');
    add_gauge_button.id = "Add_Button";
    add_gauge_button.type = 'button';
    add_gauge_button.textContent = "ADD CODE";
    add_gauge_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const codes = menu['Update Codes'];
        codes.push(`console.log(${codes.length})`);
        recompileMenus(menu_data, menu, true);
    })
    update_code_container.appendChild(add_gauge_button);
}

const createSelcWindowForm = function(form, data){
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Selection Window";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Selection_Window_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const selec_window = data['Selection Window'];
    const selc_window_container = document.createElement('div');
    selc_window_container.id = "Selection_Window_Container";
    selc_window_container.classList.add("content");
    addWindowDimensionForm(selc_window_container, data);
    addWindowStyleForm(selc_window_container, data);
    const itm_width_label = document.createElement('label');
    itm_width_label.id = "Window_Label";
    itm_width_label.textContent = `Item Width: ${selec_window['Item Width']}`;
    selc_window_container.appendChild(itm_width_label);
    const itm_width_input = document.createElement("input");
    itm_width_input.id = "Window_Input";
    itm_width_input.setAttribute("type", "text");
    itm_width_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = itm_width_input.value;
            if(isNaN(eval(value)))value = 0;
            itm_width_label.textContent = `Item Width: ${value}`;
            selection_window['Item Width'] = value;
            data['Item Width'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(itm_width_input);
    const itm_height_label = document.createElement('label');
    itm_height_label.id = "Window_Label";
    itm_height_label.textContent = `Item Height: ${selec_window['Item Height']}`;
    selc_window_container.appendChild(itm_height_label);
    const itm_height_input = document.createElement("input");
    itm_height_input.id = "Window_Input";
    itm_height_input.setAttribute("type", "text");
    itm_height_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = itm_height_input.value;
            if(isNaN(eval(value)))value = 0;
            itm_height_label.textContent = `Item Height: ${value}`;
            selection_window['Item Height'] = value;
            data['Item Height'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(itm_height_input);
    const max_cols_label = document.createElement('label');
    max_cols_label.id = "Window_Label";
    max_cols_label.textContent = `Max Columns: ${selec_window['Max Columns']}`;
    selc_window_container.appendChild(max_cols_label);
    const max_cols_input = document.createElement("input");
    max_cols_input.id = "Window_Input";
    max_cols_input.setAttribute("type", "text");
    max_cols_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = max_cols_input.value;
            if(isNaN(eval(value)))value = 0;
            max_cols_label.textContent = `Max Columns: ${value}`;
            selection_window['Max Columns'] = value;
            data['Max Columns'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(max_cols_input);
    addWindowGaugesForm(selc_window_container, data);
    addWindowSelcOptnForm(selc_window_container, data);
    const drw_name_label = document.createElement('label');
    drw_name_label.id = "Window_Label";
    drw_name_label.textContent = `Draw Name: ${selec_window['Draw Name'] ? "true" : "false"}`;
    selc_window_container.appendChild(drw_name_label);
    const drw_name_input = document.createElement("input");
    drw_name_input.id = "Window_Input";
    drw_name_input.setAttribute("type", "checkbox");
    drw_name_input.setAttribute("checked", eval(selec_window['Draw Name']));
    drw_name_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_name_input.checked;
            drw_name_label.textContent = `Draw Name: ${value}`;
            selection_window['Draw Name'] = value.toString();
            data['Draw Name'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_name_input);
    const drw_name_text_label = document.createElement('label');
    drw_name_text_label.id = "Window_Label";
    drw_name_text_label.textContent = `Name Text: %1 = Option Name`;
    selc_window_container.appendChild(drw_name_text_label);
    const drw_name_text_input = document.createElement("input");
    drw_name_text_input.id = "Window_Input";
    drw_name_text_input.setAttribute("type", "text");
    drw_name_text_input.setAttribute("value", selec_window['Name Text']);
    drw_name_text_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_name_text_input.value;
            drw_name_text_label.textContent = `Name Text: %1 = Option Name`;
            selection_window['Name Text'] = value;
            data['Name Text'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_name_text_input);
    const drw_name_x_label = document.createElement('label');
    drw_name_x_label.id = "Window_Label";
    drw_name_x_label.textContent = `Name X: ${selec_window['Name X']}`;
    selc_window_container.appendChild(drw_name_x_label);
    const drw_name_x_input = document.createElement("input");
    drw_name_x_input.id = "Window_Input";
    drw_name_x_input.setAttribute("type", "text");
    drw_name_x_input.setAttribute("value", selec_window['Name X']);
    drw_name_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_name_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_name_x_label.textContent = `Name X: ${value}`;
            selection_window['Name X'] = value;
            data['Name X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_name_x_input);
    const drw_name_y_label = document.createElement('label');
    drw_name_y_label.id = "Window_Label";
    drw_name_y_label.textContent = `Name Y: ${selec_window['Name Y']}`;
    selc_window_container.appendChild(drw_name_y_label);
    const drw_name_y_input = document.createElement("input");
    drw_name_y_input.id = "Window_Input";
    drw_name_y_input.setAttribute("type", "text");
    drw_name_y_input.setAttribute("value", selec_window['Name Y']);
    drw_name_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_name_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_name_y_label.textContent = `Name Y: ${value}`;
            selection_window['Name Y'] = value;
            data['Name Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_name_y_input);
    const drw_alt_name_label = document.createElement('label');
    drw_alt_name_label.id = "Window_Label";
    drw_alt_name_label.textContent = `Draw Alternative Name: ${selec_window['Draw Alternative Name'] ? "true" : "false"}`;
    selc_window_container.appendChild(drw_alt_name_label);
    const drw_alt_name_input = document.createElement("input");
    drw_alt_name_input.id = "Window_Input";
    drw_alt_name_input.setAttribute("type", "checkbox");
    drw_alt_name_input.setAttribute("checked", selec_window['Draw Alternative Name']);
    drw_alt_name_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_alt_name_input.checked;
            drw_alt_name_label.textContent = `Draw Alternative Name: ${value}`;
            selection_window['Draw Alternative Name'] = value;
            data['Draw Alternative Name'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_alt_name_input);
    const drw_alt_name_text_label = document.createElement('label');
    drw_alt_name_text_label.id = "Window_Label";
    drw_alt_name_text_label.textContent = `Alternative Name Text: %1 = Alternative Name`;
    selc_window_container.appendChild(drw_alt_name_text_label);
    const drw_alt_name_text_input = document.createElement("input");
    drw_alt_name_text_input.id = "Window_Input";
    drw_alt_name_text_input.setAttribute("type", "text");
    drw_alt_name_text_input.setAttribute("value", selec_window['Alternative Name Text']);
    drw_alt_name_text_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_alt_name_text_input.value;
            drw_alt_name_text_label.textContent = `Alternative Name Text: %1 = Alternative Name`;
            selection_window['Name Text'] = value;
            data['Name Text'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_alt_name_text_input);
    const drw_alt_name_x_label = document.createElement('label');
    drw_alt_name_x_label.id = "Window_Label";
    drw_alt_name_x_label.textContent = `Alternative Name X: ${selec_window['Alternative Name X']}`;
    selc_window_container.appendChild(drw_alt_name_x_label);
    const drw_alt_name_x_input = document.createElement("input");
    drw_alt_name_x_input.id = "Window_Input";
    drw_alt_name_x_input.setAttribute("type", "text");
    drw_alt_name_x_input.setAttribute("value", selec_window['Alternative Name X']);
    drw_alt_name_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_alt_name_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_alt_name_x_label.textContent = `Alternative Name X: ${value}`;
            selection_window['Alternative Name X'] = value;
            data['Alternative Name X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_alt_name_x_input);
    const drw_alt_name_y_label = document.createElement('label');
    drw_alt_name_y_label.id = "Window_Label";
    drw_alt_name_y_label.textContent = `Alternative Name Y: ${selec_window['Alternative Name Y']}`;
    selc_window_container.appendChild(drw_alt_name_y_label);
    const drw_alt_name_y_input = document.createElement("input");
    drw_alt_name_y_input.id = "Window_Input";
    drw_alt_name_y_input.setAttribute("type", "text");
    drw_alt_name_y_input.setAttribute("value", selec_window['Alternative Name Y']);
    drw_alt_name_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_alt_name_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_alt_name_y_label.textContent = `Alternative Name Y: ${value}`;
            selection_window['Alternative Name Y'] = value;
            data['Alternative Name Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_alt_name_y_input);
    const drw_desc_label = document.createElement('label');
    drw_desc_label.id = "Window_Label";
    drw_desc_label.textContent = `Draw Description: ${selec_window['Draw Description'] ? "true" : "false"}`;
    selc_window_container.appendChild(drw_desc_label);
    const drw_desc_input = document.createElement("input");
    drw_desc_input.id = "Window_Input";
    drw_desc_input.setAttribute("type", "checkbox");
    drw_desc_input.setAttribute("checked", selec_window['Draw Description']);
    drw_desc_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_desc_input.checked;
            drw_desc_label.textContent = `Draw Description: ${value}`;
            selection_window['Draw Description'] = value;
            data['Draw Description'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_desc_input);
    const drw_desc_x_label = document.createElement('label');
    drw_desc_x_label.id = "Window_Label";
    drw_desc_x_label.textContent = `Description X: ${selec_window['Description X']}`;
    selc_window_container.appendChild(drw_desc_x_label);
    const drw_desc_x_input = document.createElement("input");
    drw_desc_x_input.id = "Window_Input";
    drw_desc_x_input.setAttribute("type", "text");
    drw_desc_x_input.setAttribute("value", selec_window['Description X']);
    drw_desc_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_desc_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_desc_x_label.textContent = `Description X: ${value}`;
            selection_window['Description X'] = value;
            data['Description X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_desc_x_input);
    const drw_desc_y_label = document.createElement('label');
    drw_desc_y_label.id = "Window_Label";
    drw_desc_y_label.textContent = `Description Y: ${selec_window['Description Y']}`;
    selc_window_container.appendChild(drw_desc_y_label);
    const drw_desc_y_input = document.createElement("input");
    drw_desc_y_input.id = "Window_Input";
    drw_desc_y_input.setAttribute("type", "text");
    drw_desc_y_input.setAttribute("value", selec_window['Description Y']);
    drw_desc_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_desc_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_desc_y_label.textContent = `Description Y: ${value}`;
            selection_window['Description Y'] = value;
            data['Description Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_desc_y_input);
    const drw_pic_label = document.createElement('label');
    drw_pic_label.id = "Window_Label";
    drw_pic_label.textContent = `Draw Picture: ${selec_window['Draw Picture'] ? "true" : "false"}`;
    selc_window_container.appendChild(drw_pic_label);
    const drw_pic_input = document.createElement("input");
    drw_pic_input.id = "Window_Input";
    drw_pic_input.setAttribute("type", "checkbox");
    drw_pic_input.setAttribute("checked", selec_window['Draw Picture']);
    drw_pic_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_pic_input.checked;
            drw_pic_label.textContent = `Draw Picture: ${value}`;
            selection_window['Draw Picture'] = value;
            data['Draw Picture'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_pic_input);
    const drw_pic_indx_label = document.createElement('label');
    drw_pic_indx_label.id = "Window_Label";
    drw_pic_indx_label.textContent = `Picture Index: ${selec_window['Picture Index']}`;
    selc_window_container.appendChild(drw_pic_indx_label);
    const drw_pic_indx_input = document.createElement("input");
    drw_pic_indx_input.id = "Window_Input";
    drw_pic_indx_input.setAttribute("type", "text");
    drw_pic_indx_input.setAttribute("value", selec_window['Picture Index']);
    drw_pic_indx_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_pic_indx_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_pic_indx_label.textContent = `Picture Index: ${value}`;
            selection_window['Picture Index'] = value;
            data['Picture Index'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_pic_indx_input);
    const drw_pic_x_label = document.createElement('label');
    drw_pic_x_label.id = "Window_Label";
    drw_pic_x_label.textContent = `Picture X: ${selec_window['Picture X']}`;
    selc_window_container.appendChild(drw_pic_x_label);
    const drw_pic_x_input = document.createElement("input");
    drw_pic_x_input.id = "Window_Input";
    drw_pic_x_input.setAttribute("type", "text");
    drw_pic_x_input.setAttribute("value", selec_window['Picture X']);
    drw_pic_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_pic_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_pic_x_label.textContent = `Picture X: ${value}`;
            selection_window['Picture X'] = value;
            data['Picture X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_pic_x_input);
    const drw_pic_y_label = document.createElement('label');
    drw_pic_y_label.id = "Window_Label";
    drw_pic_y_label.textContent = `Picture Y: ${selec_window['Picture Y']}`;
    selc_window_container.appendChild(drw_pic_y_label);
    const drw_pic_y_input = document.createElement("input");
    drw_pic_y_input.id = "Window_Input";
    drw_pic_y_input.setAttribute("type", "text");
    drw_pic_y_input.setAttribute("value", selec_window['Picture Y']);
    drw_pic_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_pic_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_pic_y_label.textContent = `Picture Y: ${value}`;
            selection_window['Picture Y'] = value;
            data['Picture Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_pic_y_input);
    const drw_pic_w_label = document.createElement('label');
    drw_pic_w_label.id = "Window_Label";
    drw_pic_w_label.textContent = `Picture Width: ${selec_window['Picture Width']}`;
    selc_window_container.appendChild(drw_pic_w_label);
    const drw_pic_w_input = document.createElement("input");
    drw_pic_w_input.id = "Window_Input";
    drw_pic_w_input.setAttribute("type", "text");
    drw_pic_w_input.setAttribute("value", selec_window['Picture Width']);
    drw_pic_w_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_pic_w_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_pic_w_label.textContent = `Picture Width: ${value}`;
            selection_window['Picture Width'] = value;
            data['Picture Width'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_pic_w_input);
    const drw_pic_h_label = document.createElement('label');
    drw_pic_h_label.id = "Window_Label";
    drw_pic_h_label.textContent = `Picture Height: ${selec_window['Picture Height']}`;
    selc_window_container.appendChild(drw_pic_h_label);
    const drw_pic_h_input = document.createElement("input");
    drw_pic_h_input.id = "Window_Input";
    drw_pic_h_input.setAttribute("type", "text");
    drw_pic_h_input.setAttribute("value", selec_window['Picture Height']);
    drw_pic_h_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Selection Window'];
        if(selection_window){
            let value = drw_pic_h_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_pic_h_label.textContent = `Picture Height: ${value}`;
            selection_window['Picture Height'] = value;
            data['Picture Height'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_pic_h_input);
    form.appendChild(selc_window_container);
}

const createSelcDataWindowsForm = function(form, data){
    const data_windows = data['Selection Data Windows'];
    let index = 0;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Selection Data Windows";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Selection_Data_Window_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const windows_container = document.createElement('div');
    windows_container.id = "Selection_Data_Window_Container";
    windows_container.classList.add("content");
    form.appendChild(windows_container);
    data_windows.forEach((data_window)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${data_window['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Data_Window_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        windows_container.appendChild(collapse_button);
        const selec_window = data['Selection Data Windows'][index];
        const selc_window_container = document.createElement('div');
        selc_window_container.id = "Data_Window_Container";
        selc_window_container.classList.add("content");
        windows_container.appendChild(selc_window_container);
        const window_name_label = document.createElement('label');
        window_name_label.id = "Window_Label";
        window_name_label.textContent = `Window Name: ${data_window['Name']}`;
        selc_window_container.appendChild(window_name_label);
        const window_name_input = document.createElement("input");
        window_name_input.id = "Window_Input";
        window_name_input.setAttribute("value", `${data_window['Name']}`);
        window_name_input._saved_index = JSON.parse(JSON.stringify(index));
        window_name_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = window_name_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                const value = window_name_input.value;
                collapse_button.textContent = `${value}`
                window_name_label.textContent = `Window Name: ${value}`;
                selection_window['Name'] = value.toString();
                data_window['Name'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(window_name_input);
        addWindowDataDimensionForm(selc_window_container, data, index);
        addWindowDataStyleForm(selc_window_container, data, index);
        addWindowDataRequirementsForm(selc_window_container, data, index);
        addWindowDataGaugesForm(selc_window_container, data, index);
        const drw_name_label = document.createElement('label');
        drw_name_label.id = "Window_Label";
        drw_name_label.textContent = `Draw Option Name: ${data_window['Draw Option Name'] ? "true" : "false"}`;
        selc_window_container.appendChild(drw_name_label);
        const drw_name_input = document.createElement("input");
        drw_name_input.id = "Window_Input";
        drw_name_input.setAttribute("type", "checkbox");
        drw_name_input.setAttribute("checked", eval(data_window['Draw Option Name']));
        drw_name_input.checked = eval(data_window['Draw Option Name']);
        drw_name_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_name_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_name_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_name_input.checked;
                drw_name_label.textContent = `Draw Option Name: ${value}`;
                selection_window['Draw Option Name'] = value.toString();
                data_window['Draw Option Name'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_name_input);
        const drw_name_text_label = document.createElement('label');
        drw_name_text_label.id = "Window_Label";
        drw_name_text_label.textContent = `Name Text: %1 = Option Name`;
        selc_window_container.appendChild(drw_name_text_label);
        const drw_name_text_input = document.createElement("input");
        drw_name_text_input.id = "Window_Input";
        drw_name_text_input.setAttribute("type", "text");
        drw_name_text_input.setAttribute("value", data_window['Name Text']);
        drw_name_text_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_name_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_name_text_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_name_text_input.value;
                drw_name_text_label.textContent = `Name Text: %1 = Option Name`;
                selection_window['Name Text'] = value;
                data_window['Name Text'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_name_text_input);
        const drw_name_x_label = document.createElement('label');
        drw_name_x_label.id = "Window_Label";
        drw_name_x_label.textContent = `Name X: ${data_window['Name X']}`;
        selc_window_container.appendChild(drw_name_x_label);
        const drw_name_x_input = document.createElement("input");
        drw_name_x_input.id = "Window_Input";
        drw_name_x_input.setAttribute("type", "text");
        drw_name_x_input.setAttribute("value", data_window['Name X']);
        drw_name_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_name_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_name_x_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_name_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_name_x_label.textContent = `Name X: ${value}`;
                selection_window['Name X'] = value;
                data_window['Name X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_name_x_input);
        const drw_name_y_label = document.createElement('label');
        drw_name_y_label.id = "Window_Label";
        drw_name_y_label.textContent = `Name Y: ${data_window['Name Y']}`;
        selc_window_container.appendChild(drw_name_y_label);
        const drw_name_y_input = document.createElement("input");
        drw_name_y_input.id = "Window_Input";
        drw_name_y_input.setAttribute("type", "text");
        drw_name_y_input.setAttribute("value", data_window['Name Y']);
        drw_name_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_name_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_name_y_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_name_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_name_y_label.textContent = `Name Y: ${value}`;
                selection_window['Name Y'] = value;
                data_window['Name Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_name_y_input);
        const drw_alt_name_label = document.createElement('label');
        drw_alt_name_label.id = "Window_Label";
        drw_alt_name_label.textContent = `Draw Alternative Name: ${data_window['Draw Alternative Name'] ? "true" : "false"}`;
        selc_window_container.appendChild(drw_alt_name_label);
        const drw_alt_name_input = document.createElement("input");
        drw_alt_name_input.id = "Window_Input";
        drw_alt_name_input.setAttribute("type", "checkbox");
        drw_alt_name_input.setAttribute("checked", eval(data_window['Draw Alternative Name']));
        drw_alt_name_input.checked = eval(data_window['Draw Alternative Name']);
        drw_alt_name_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_alt_name_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_alt_name_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_alt_name_input.checked;
                drw_alt_name_label.textContent = `Draw Alternative Name: ${value}`;
                selection_window['Draw Alternative Name'] = value;
                data_window['Draw Alternative Name'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_alt_name_input);
        const drw_alt_name_text_label = document.createElement('label');
        drw_alt_name_text_label.id = "Window_Label";
        drw_alt_name_text_label.textContent = `Alternative Name Text: %1 = Alternative Name`;
        selc_window_container.appendChild(drw_alt_name_text_label);
        const drw_alt_name_text_input = document.createElement("input");
        drw_alt_name_text_input.id = "Window_Input";
        drw_alt_name_text_input.setAttribute("type", "text");
        drw_alt_name_text_input.setAttribute("value", data_window['Alternative Name Text']);
        drw_alt_name_text_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_alt_name_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_alt_name_text_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_alt_name_text_input.value;
                drw_alt_name_text_label.textContent = `Alternative Name Text: %1 = Alternative Name`;
                selection_window['Name Text'] = value;
                data_window['Name Text'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_alt_name_text_input);
        const drw_alt_name_x_label = document.createElement('label');
        drw_alt_name_x_label.id = "Window_Label";
        drw_alt_name_x_label.textContent = `Alternative Name X: ${data_window['Alternative Name X']}`;
        selc_window_container.appendChild(drw_alt_name_x_label);
        const drw_alt_name_x_input = document.createElement("input");
        drw_alt_name_x_input.id = "Window_Input";
        drw_alt_name_x_input.setAttribute("type", "text");
        drw_alt_name_x_input.setAttribute("value", data_window['Alternative Name X']);
        drw_alt_name_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_alt_name_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_alt_name_x_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_alt_name_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_alt_name_x_label.textContent = `Alternative Name X: ${value}`;
                selection_window['Alternative Name X'] = value;
                data_window['Alternative Name X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_alt_name_x_input);
        const drw_alt_name_y_label = document.createElement('label');
        drw_alt_name_y_label.id = "Window_Label";
        drw_alt_name_y_label.textContent = `Alternative Name Y: ${data_window['Alternative Name Y']}`;
        selc_window_container.appendChild(drw_alt_name_y_label);
        const drw_alt_name_y_input = document.createElement("input");
        drw_alt_name_y_input.id = "Window_Input";
        drw_alt_name_y_input.setAttribute("type", "text");
        drw_alt_name_y_input.setAttribute("value", data_window['Alternative Name Y']);
        drw_alt_name_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_alt_name_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_alt_name_y_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_alt_name_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_alt_name_y_label.textContent = `Alternative Name Y: ${value}`;
                selection_window['Alternative Name Y'] = value;
                data_window['Alternative Name Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_alt_name_y_input);
        const drw_desc_label = document.createElement('label');
        drw_desc_label.id = "Window_Label";
        drw_desc_label.textContent = `Draw Description: ${data_window['Draw Description'] ? "true" : "false"}`;
        selc_window_container.appendChild(drw_desc_label);
        const drw_desc_input = document.createElement("input");
        drw_desc_input.id = "Window_Input";
        drw_desc_input.setAttribute("type", "checkbox");
        drw_desc_input.setAttribute("checked", eval(data_window['Draw Description']));
        drw_desc_input.checked = eval(data_window['Draw Description']);
        drw_desc_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_desc_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_desc_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_desc_input.checked;
                drw_desc_label.textContent = `Draw Description: ${value}`;
                selection_window['Draw Description'] = value;
                data_window['Draw Description'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_desc_input);
        const drw_desc_x_label = document.createElement('label');
        drw_desc_x_label.id = "Window_Label";
        drw_desc_x_label.textContent = `Description X: ${data_window['Description X']}`;
        selc_window_container.appendChild(drw_desc_x_label);
        const drw_desc_x_input = document.createElement("input");
        drw_desc_x_input.id = "Window_Input";
        drw_desc_x_input.setAttribute("type", "text");
        drw_desc_x_input.setAttribute("value", data_window['Description X']);
        drw_desc_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_desc_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_desc_x_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_desc_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_desc_x_label.textContent = `Description X: ${value}`;
                selection_window['Description X'] = value;
                data_window['Description X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_desc_x_input);
        const drw_desc_y_label = document.createElement('label');
        drw_desc_y_label.id = "Window_Label";
        drw_desc_y_label.textContent = `Description Y: ${data_window['Description Y']}`;
        selc_window_container.appendChild(drw_desc_y_label);
        const drw_desc_y_input = document.createElement("input");
        drw_desc_y_input.id = "Window_Input";
        drw_desc_y_input.setAttribute("type", "text");
        drw_desc_y_input.setAttribute("value", data_window['Description Y']);
        drw_desc_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_desc_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_desc_y_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_desc_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_desc_y_label.textContent = `Description Y: ${value}`;
                selection_window['Description Y'] = value;
                data_window['Description Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_desc_y_input);
        const drw_pic_label = document.createElement('label');
        drw_pic_label.id = "Window_Label";
        drw_pic_label.textContent = `Draw Picture: ${eval(data_window['Draw Picture']) ? "true" : "false"}`;
        selc_window_container.appendChild(drw_pic_label);
        const drw_pic_input = document.createElement("input");
        drw_pic_input.id = "Window_Input";
        drw_pic_input.setAttribute("type", "checkbox");
        drw_pic_input.setAttribute("checked", eval(data_window['Draw Picture']));
        drw_pic_input.checked = eval(data_window['Draw Picture']);
        drw_pic_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_pic_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_pic_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_pic_input.checked;
                drw_pic_label.textContent = `Draw Picture: ${value}`;
                selection_window['Draw Picture'] = value;
                data_window['Draw Picture'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_pic_input);
        const drw_pic_indx_label = document.createElement('label');
        drw_pic_indx_label.id = "Window_Label";
        drw_pic_indx_label.textContent = `Picture Index: ${data_window['Picture Index']}`;
        selc_window_container.appendChild(drw_pic_indx_label);
        const drw_pic_indx_input = document.createElement("input");
        drw_pic_indx_input.id = "Window_Input";
        drw_pic_indx_input.setAttribute("type", "text");
        drw_pic_indx_input.setAttribute("value", data_window['Picture Index']);
        drw_pic_indx_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_pic_indx_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_pic_indx_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_pic_indx_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_pic_indx_label.textContent = `Picture Index: ${value}`;
                selection_window['Picture Index'] = value;
                data_window['Picture Index'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_pic_indx_input);
        const drw_pic_x_label = document.createElement('label');
        drw_pic_x_label.id = "Window_Label";
        drw_pic_x_label.textContent = `Picture X: ${data_window['Picture X']}`;
        selc_window_container.appendChild(drw_pic_x_label);
        const drw_pic_x_input = document.createElement("input");
        drw_pic_x_input.id = "Window_Input";
        drw_pic_x_input.setAttribute("type", "text");
        drw_pic_x_input.setAttribute("value", data_window['Picture X']);
        drw_pic_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_pic_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_pic_x_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_pic_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_pic_x_label.textContent = `Picture X: ${value}`;
                selection_window['Picture X'] = value;
                data_window['Picture X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_pic_x_input);
        const drw_pic_y_label = document.createElement('label');
        drw_pic_y_label.id = "Window_Label";
        drw_pic_y_label.textContent = `Picture Y: ${data_window['Picture Y']}`;
        selc_window_container.appendChild(drw_pic_y_label);
        const drw_pic_y_input = document.createElement("input");
        drw_pic_y_input.id = "Window_Input";
        drw_pic_y_input.setAttribute("type", "text");
        drw_pic_y_input.setAttribute("value", data_window['Picture Y']);
        drw_pic_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_pic_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_pic_y_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_pic_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_pic_y_label.textContent = `Picture Y: ${value}`;
                selection_window['Picture Y'] = value;
                data_window['Picture Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_pic_y_input);
        const drw_pic_w_label = document.createElement('label');
        drw_pic_w_label.id = "Window_Label";
        drw_pic_w_label.textContent = `Picture Width: ${data_window['Picture Width']}`;
        selc_window_container.appendChild(drw_pic_w_label);
        const drw_pic_w_input = document.createElement("input");
        drw_pic_w_input.id = "Window_Input";
        drw_pic_w_input.setAttribute("type", "text");
        drw_pic_w_input.setAttribute("value", data_window['Picture Width']);
        drw_pic_w_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_pic_w_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_pic_w_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_pic_w_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_pic_w_label.textContent = `Picture Width: ${value}`;
                selection_window['Picture Width'] = value;
                data_window['Picture Width'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_pic_w_input);
        const drw_pic_h_label = document.createElement('label');
        drw_pic_h_label.id = "Window_Label";
        drw_pic_h_label.textContent = `Picture Height: ${data_window['Picture Height']}`;
        selc_window_container.appendChild(drw_pic_h_label);
        const drw_pic_h_input = document.createElement("input");
        drw_pic_h_input.id = "Window_Input";
        drw_pic_h_input.setAttribute("type", "text");
        drw_pic_h_input.setAttribute("value", data_window['Picture Height']);
        drw_pic_h_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_pic_h_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_pic_h_input._saved_index;
            const selection_window = menu['Selection Data Windows'][index];
            if(selection_window){
                let value = drw_pic_h_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_pic_h_label.textContent = `Picture Height: ${value}`;
                selection_window['Picture Height'] = value;
                data_window['Picture Height'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_pic_h_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button._saved_index = JSON.parse(JSON.stringify(index));
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = delete_button._saved_index;
            if(index >= 0){
                menu['Selection Data Windows'].splice(index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        selc_window_container.appendChild(delete_button);
        index++;
    })
    const add_window_button = document.createElement('button');
    add_window_button.id = "Add_Button";
    add_window_button.type = 'button';
    add_window_button.textContent = "ADD SELECTION DATA WINDOW";
    add_window_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const windows = menu['Selection Data Windows'];
        let new_index = windows.length;
        const new_window = {
            "Name":`New Window:${new_index}`,
            "Dimension Configuration": {
                "X": "0", 
                "Y": "0", 
                "Width": "1", 
                "Height": "1"
            },
            "Gauges": [],
            "Draw Description": `false`,
            "Description X": "0",
            "Description Y": "0",
            "Draw Picture": `false`,
            "Picture Index": "1",
            "Picture X": "0",
            "Picture Y": "0",
            "Picture Width": "0",
            "Picture Height": "0",
            "Draw Option Name": `false`,
            "Name Text": "%1",
            "Name X": "0",
            "Name Y": "0",
            "Draw Alternative Option Name": `false`,
            "Alternative Name Text": "%1",
            "Alternative Name X": "0",
            "Alternative Name Y": "0",
            "Window Font and Style Configuration": {
                "Font Settings": "", 
                "Font Size": "16", 
                "Font Face": "sans-serif", 
                "Base Font Color": "#ffffff", 
                "Font Outline Color": "rgba(0, 0, 0, 0.5)",
                "Font Outline Thickness": "3",
                "Window Skin": "Window",
                "Window Opacity": "255",
                "Show Window Dimmer": "false",
            }
        }
        windows.push(new_window);
        recompileMenus(menu_data, menu, true);
    })
    windows_container.appendChild(add_window_button);
}

const addWindowBasicTextsForm = function(container, data, index){
    const selection_windows = data['Basic Windows'];
    const selection_window = selection_windows[index];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Draw Texts";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Text_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const text_config_container = document.createElement('div');
    text_config_container.id = "Text_Config_Container";
    text_config_container.classList.add("content");
    container.appendChild(text_config_container);
    const existing_texts = selection_window['Draw Texts'] || [];
    let sav_index = 0;
    existing_texts.forEach((text_data)=>{
        const text_container = document.createElement('div');
        text_container.id = "Text_Container";
        text_config_container.appendChild(text_container);
        const draw_text_label = document.createElement('label');
        draw_text_label.id = "Window_Label";
        draw_text_label.textContent = `Text:`;
        text_container.appendChild(draw_text_label);
        const draw_text_input = document.createElement('input');
        draw_text_input.setAttribute("value", text_data['Text']);
        draw_text_input._saved_index = JSON.parse(JSON.stringify(sav_index));
        draw_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = draw_text_input._saved_index;
            if(sav_index >= 0){
                const value = draw_text_input.value;
                menu['Basic Windows'][index]['Draw Texts'][sav_index]['Text'] = value;
                draw_text_label.textContent = `Text:`;
                text_data['Text'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        text_container.appendChild(draw_text_input);
        const draw_text_x_label = document.createElement('label');
        draw_text_x_label.id = "Window_Label";
        draw_text_x_label.textContent = `X: ${text_data['X']}`;
        text_container.appendChild(draw_text_x_label);
        const draw_text_x_input = document.createElement('input');
        draw_text_x_input.setAttribute("value", text_data['X']);
        draw_text_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = draw_text_input._saved_index;
            if(sav_index >= 0){
                let value = draw_text_x_input.value;
                if(isNaN(eval(value)))value = 0;
                menu['Basic Windows'][index]['Draw Texts'][sav_index]['X'] = value;
                draw_text_x_label.textContent = `X:`;
                text_data['X'] = value;
                collapse_button.textContent = value;
                recompileMenus(menu_data, menu);
            }
        })
        text_container.appendChild(draw_text_x_input);
        const draw_text_y_label = document.createElement('label');
        draw_text_y_label.id = "Window_Label";
        draw_text_y_label.textContent = `Y: ${text_data['Y']}`;
        text_container.appendChild(draw_text_y_label);
        const draw_text_y_input = document.createElement('input');
        draw_text_y_input.setAttribute("value", text_data['Y']);
        draw_text_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = draw_text_input._saved_index;
            if(sav_index >= 0){
                let value = draw_text_y_input.value;
                if(isNaN(eval(value)))value = 0;
                menu['Basic Windows'][index]['Draw Texts'][sav_index]['Y'] = value;
                draw_text_y_label.textContent = `Y: ${value}`;
                text_data['Y'] = value;
                collapse_button.textContent = value;
                recompileMenus(menu_data, menu);
            }
        })
        text_container.appendChild(draw_text_y_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const del_index = draw_text_input._saved_index;
            if(del_index >= 0){
                menu['Basic Windows'][index]['Draw Texts'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        text_container.appendChild(delete_button);
        sav_index++;
    })
    const add_text_button = document.createElement('button');
    add_text_button.id = "Add_Button";
    add_text_button.type = 'button';
    add_text_button.textContent = "ADD TEXT";
    add_text_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const texts = menu['Basic Windows'][index]['Draw Texts'];
        const new_index = texts[(texts.length - 1)];
        const text_obj = {
            "Text":`${new_index}`,
            "X":"0",
            "Y":"0"
        }
        texts.push(text_obj);
        recompileMenus(menu_data, menu, true);
    })
    text_config_container.appendChild(add_text_button);
}

const addWindowBasicTextRefsForm = function(container, data, index){
    const selection_windows = data['Basic Windows'];
    const selection_window = selection_windows[index];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Text References";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Ref_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const text_config_container = document.createElement('div');
    text_config_container.id = "Ref_Config_Container";
    text_config_container.classList.add("content");
    container.appendChild(text_config_container);
    const existing_refs = selection_window['Text References'] || [];
    let sav_index = 0;
    console.log(existing_refs)
    existing_refs.forEach((reference)=>{
        const ref_container = document.createElement('div');
        ref_container.id = "Ref_Container";
        text_config_container.appendChild(ref_container);
        const ref_label = document.createElement('label');
        ref_label.id = "Window_Label";
        ref_label.textContent = `Reference:`;
        ref_container.appendChild(ref_label);
        const ref_input = document.createElement('input');
        ref_input.setAttribute("value", reference);
        ref_input._saved_index = JSON.parse(JSON.stringify(sav_index));
        ref_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = ref_input._saved_index;
            if(sav_index >= 0){
                if(ref_input.value){
                    const value = ref_input.value;
                    menu['Basic Windows'][index]['Text References'][sav_index] = value;
                    draw_text_label.textContent = `Reference:`;
                    reference = value;
                    collapse_button.textContent = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        ref_container.appendChild(ref_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Basic Windows'][index];
            let del_index = ref_input._saved_index;
            for(let i = 0; i < selection_window['Text References'].length; i++){
                const data = selection_window['Text References'][i];
                if(data){
                    if(data == reference){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Basic Windows'][index]['Text References'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        ref_container.appendChild(delete_button);
        sav_index++;
    })
    const add_text_button = document.createElement('button');
    add_text_button.id = "Add_Button";
    add_text_button.type = 'button';
    add_text_button.textContent = "ADD REFERENCE";
    add_text_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const refs = menu['Basic Windows'][index]['Text References'];
        const new_index = refs.length;
        refs.push(`Reference:${new_index}`);
        recompileMenus(menu_data, menu, true);
    })
    text_config_container.appendChild(add_text_button);
}

const addWindowBasicPicturesForm = function(container, data, index){
    const selection_windows = data['Basic Windows'];
    const selection_window = selection_windows[index];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Draw Pictures";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Pic_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const picture_config_container = document.createElement('div');
    picture_config_container.id = "Pic_Config_Container";
    picture_config_container.classList.add("content");
    container.appendChild(picture_config_container);
    const existing_pics = selection_window['Draw Pictures'] || [];
    let sav_index = 0;
    existing_pics.forEach((pic_data)=>{
        const pic_container = document.createElement('div');
        pic_container.id = "Picture_Container";
        picture_config_container.appendChild(pic_container);
        const picture_label = document.createElement('label');
        picture_label.id = "Window_Label";
        picture_label.textContent = `Picture: ${pic_data['Picture']}`;
        pic_container.appendChild(picture_label);
        const picture_input = document.createElement('input');
        picture_input.setAttribute("value", pic_data['Picture']);
        picture_input.setAttribute("type", "file");
        picture_input.setAttribute("accept", "image/png");
        picture_input._saved_index = JSON.parse(JSON.stringify(sav_index));
        picture_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = picture_input._saved_index;
            if(sav_index >= 0){
                if(picture_input.value){
                    const input_paths = (picture_input.value || "").match(/(img\\pictures\\[a-z\d*|\W\D*]+)/gm);
                    if(input_paths.length > 0){
                        const path = input_paths[0];
                        let file_name = path.replace("img\\pictures\\", "");
                        file_name = file_name.replace(".png", "");
                        file_name = file_name.replace("\\", "/");
                        menu['Basic Windows'][index]['Draw Pictures'][sav_index]['Picture'] = file_name;
                        picture_label.textContent = `Picture: ${file_name}`;
                        pic_data['Picture'] = file_name;
                    }
                    recompileMenus(menu_data, menu);
                }
            }
        })
        pic_container.appendChild(picture_input);
        const draw_text_x_label = document.createElement('label');
        draw_text_x_label.id = "Window_Label";
        draw_text_x_label.textContent = `X: ${pic_data['X']}`;
        pic_container.appendChild(draw_text_x_label);
        const draw_text_x_input = document.createElement('input');
        draw_text_x_input.setAttribute("value", pic_data['X']);
        draw_text_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = picture_input._saved_index;
            if(sav_index >= 0){
                let value = draw_text_x_input.value;
                if(isNaN(eval(value)))value = 0;
                menu['Basic Windows'][index]['Draw Pictures'][sav_index]['X'] = value;
                draw_text_x_label.textContent = `X:`;
                pic_data['X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        pic_container.appendChild(draw_text_x_input);
        const draw_text_y_label = document.createElement('label');
        draw_text_y_label.id = "Window_Label";
        draw_text_y_label.textContent = `Y: ${pic_data['Y']}`;
        pic_container.appendChild(draw_text_y_label);
        const draw_text_y_input = document.createElement('input');
        draw_text_y_input.setAttribute("value", pic_data['Y']);
        draw_text_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = picture_input._saved_index;
            if(sav_index >= 0){
                let value = draw_text_y_input.value;
                if(isNaN(eval(value)))value = 0;
                menu['Basic Windows'][index]['Draw Pictures'][sav_index]['Y'] = value;
                draw_text_y_label.textContent = `Y: ${value}`;
                pic_data['Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        pic_container.appendChild(draw_text_y_input);
        const draw_text_w_label = document.createElement('label');
        draw_text_w_label.id = "Window_Label";
        draw_text_w_label.textContent = `Width: ${pic_data['Width']}`;
        pic_container.appendChild(draw_text_w_label);
        const draw_text_w_input = document.createElement('input');
        draw_text_w_input.setAttribute("value", pic_data['Width']);
        draw_text_w_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = picture_input._saved_index;
            if(sav_index >= 0){
                let value = draw_text_w_input.value;
                if(isNaN(eval(value)))value = 0;
                menu['Basic Windows'][index]['Draw Pictures'][sav_index]['Width'] = value;
                draw_text_w_label.textContent = `Width: ${value}`;
                pic_data['Width'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        pic_container.appendChild(draw_text_w_input);
        const draw_text_h_label = document.createElement('label');
        draw_text_h_label.id = "Window_Label";
        draw_text_h_label.textContent = `Height: ${pic_data['Height']}`;
        pic_container.appendChild(draw_text_h_label);
        const draw_text_h_input = document.createElement('input');
        draw_text_h_input.setAttribute("value", pic_data['Height']);
        draw_text_h_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const sav_index = picture_input._saved_index;
            if(sav_index >= 0){
                let value = draw_text_h_input.value;
                if(isNaN(eval(value)))value = 0;
                menu['Basic Windows'][index]['Draw Pictures'][sav_index]['Height'] = value;
                draw_text_h_label.textContent = `Height: ${value}`;
                pic_data['Height'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        pic_container.appendChild(draw_text_h_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const del_index = picture_input._saved_index;
            if(del_index >= 0){
                menu['Basic Windows'][index]['Draw Pictures'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        pic_container.appendChild(delete_button);
        sav_index++;
    })
    const add_pic_button = document.createElement('button');
    add_pic_button.id = "Add_Button";
    add_pic_button.type = 'button';
    add_pic_button.textContent = "ADD PICTURE";
    add_pic_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const texts = menu['Basic Windows'][index]['Draw Pictures'];
        const last_gauge = texts[texts.length - 1];
        let new_index = 0;
        if(last_gauge){
            new_index = eval(last_gauge['ID']) + 1;
        }
        const text_obj = {
            "Picture":``,
            "X":"0",
            "Y":"0",
            "Width":"0",
            "Height":"0"
        }
        texts.push(text_obj);
        recompileMenus(menu_data, menu, true);
    })
    picture_config_container.appendChild(add_pic_button);
}

const createBasicWindowsForm = function(form, data){
    const data_windows = data['Basic Windows'];
    let index = 0;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Basic Windows";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Selection_Data_Window_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const windows_container = document.createElement('div');
    windows_container.id = "Selection_Data_Window_Container";
    windows_container.classList.add("content");
    form.appendChild(windows_container);
    data_windows.forEach((data_window)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${data_window['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Data_Window_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        windows_container.appendChild(collapse_button);
        const selc_window_container = document.createElement('div');
        selc_window_container.id = "Data_Window_Container";
        selc_window_container.classList.add("content");
        windows_container.appendChild(selc_window_container);
        const window_name_label = document.createElement('label');
        window_name_label.id = "Window_Label";
        window_name_label.textContent = `Window Name: ${data_window['Name']}`;
        selc_window_container.appendChild(window_name_label);
        const window_name_input = document.createElement("input");
        window_name_input.id = "Window_Input";
        window_name_input.setAttribute("value", `${data_window['Name']}`);
        window_name_input._saved_index = JSON.parse(JSON.stringify(index));
        window_name_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = window_name_input._saved_index;
            const selection_window = menu['Basic Windows'][index];
            if(selection_window){
                const value = window_name_input.value;
                collapse_button.textContent = `${value}`
                window_name_label.textContent = `Window Name: ${value}`;
                selection_window['Name'] = value.toString();
                data_window['Name'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(window_name_input);
        addWindowBasicDimensionForm(selc_window_container, data, index);
        addWindowBasicStyleForm(selc_window_container, data, index);
        addWindowBasicRequirementsForm(selc_window_container, data, index);
        addWindowBasicGaugesForm(selc_window_container, data, index);
        addWindowBasicTextsForm(selc_window_container, data, index);
        addWindowBasicTextRefsForm(selc_window_container, data, index);
        addWindowBasicPicturesForm(selc_window_container, data, index);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button._saved_index = JSON.parse(JSON.stringify(index));
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = delete_button._saved_index;
            if(index >= 0){
                menu['Basic Windows'].splice(index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        selc_window_container.appendChild(delete_button);
        index++;
    })
    const add_window_button = document.createElement('button');
    add_window_button.id = "Add_Button";
    add_window_button.type = 'button';
    add_window_button.textContent = "ADD BASIC WINDOW";
    add_window_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const windows = menu['Basic Windows'];
        let new_index = windows.length;
        const new_window = {
            "Name":`New Window:${new_index}`,
            "Dimension Configuration": {
                "X": "0", 
                "Y": "0", 
                "Width": "1", 
                "Height": "1"
            },
            "Draw Pictures": [],
            "Draw Texts": [],
            "Text References": [],
            "Gauges": [],
            "Window Font and Style Configuration": {
                "Font Settings": "", 
                "Font Size": "16", 
                "Font Face": "sans-serif", 
                "Base Font Color": "#ffffff", 
                "Font Outline Color": "rgba(0, 0, 0, 0.5)",
                "Font Outline Thickness": "3",
                "Window Skin": "Window",
                "Window Opacity": "255",
                "Show Window Dimmer": "false",
            }
        }
        windows.push(new_window);
        recompileMenus(menu_data, menu, true);
    })
    windows_container.appendChild(add_window_button);
}

const addActrSelcBaseParamForm = function(container, data){
    const selection_window = data['Actor Selection Window'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Draw Base Params";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Param_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const param_config_container = document.createElement('div');
    param_config_container.id = "Param_Config_Container";
    param_config_container.classList.add("content");
    container.appendChild(param_config_container);
    const params = selection_window['Draw Base Params'] || [];
    params.forEach((param_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${param_data['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Param_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        param_config_container.appendChild(collapse_button);
        const param_div = document.createElement('div');
        param_div.id = "Param_Container";
        param_div.classList.add("content");
        param_config_container.appendChild(param_div);
        const param_name_label = document.createElement('label');
        param_name_label.id = "Window_Label";
        param_name_label.textContent = `Name: ${param_data['Name']}`;
        param_div.appendChild(param_name_label);
        const param_name_input = document.createElement('input');
        param_name_input.setAttribute("value", param_data['Name']);
        param_name_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_name_input.value){
                    const new_label = param_name_input.value;
                    menu['Actor Selection Window']['Draw Base Params'][sav_index]['Name'] = new_label;
                    param_name_label.textContent = `Name: ${new_label}`;
                    param_data['Name'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_name_input);
        const param_id_label = document.createElement('label');
        param_id_label.id = "Window_Label";
        param_id_label.textContent = `Base Param ID: ${param_data['Base Param']}`;
        param_div.appendChild(param_id_label);
        const param_id_input = document.createElement('input');
        param_id_input.setAttribute("value", param_data['Base Param']);
        param_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_id_input.value){
                    let value = param_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Draw Base Params'][sav_index]['Base Param'] = value.toString();
                    param_name_label.textContent = `Base Param ID: ${value}`;
                    param_data['Base Param'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_id_input);
        const param_text_label = document.createElement('label');
        param_text_label.id = "Window_Label";
        param_text_label.textContent = "Param Text: %1 = Value";
        param_div.appendChild(param_text_label);
        const param_text_input = document.createElement('input');
        param_text_input.id = "Window_Input";
        param_text_input.setAttribute("value", param_data['Param Text']);
        param_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_text_input.value){
                    let value = param_text_input.value;
                    menu['Actor Selection Window']['Draw Base Params'][sav_index]['Param Text'] = value.toString();
                    param_name_label.textContent = `Param Text: %1 = Value`;
                    param_data['Param Text'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_text_input)
        const param_x_label = document.createElement(`label`);
        param_x_label.id = "Window_Label";
        param_x_label.textContent = `X: ${param_data['X']}`;
        param_div.appendChild(param_x_label);
        const param_x_input = document.createElement("input");
        param_x_input.id = "Window_Input";
        param_x_input.setAttribute("value", param_data['X']);
        param_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_x_input.value){
                    let value = param_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Draw Base Params'][sav_index]['X'] = value;
                    param_x_label.textContent = `X: ${value}`;
                    param_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_x_input);
        const param_y_label = document.createElement(`label`);
        param_y_label.id = "Window_Label";
        param_y_label.textContent = `Y: ${param_data['Y']}`;
        param_div.appendChild(param_y_label);
        const param_y_input = document.createElement("input");
        param_y_input.id = "Window_Input";
        param_y_input.setAttribute("value", param_data['Y']);
        param_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_y_input.value){
                    let value = param_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Draw Base Params'][sav_index]['Y'] = value;
                    param_y_label.textContent = `Y: ${value}`;
                    param_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_y_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let del_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Actor Selection Window']['Draw Base Params'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        param_div.appendChild(delete_button);
    })
    const add_param_button = document.createElement('button');
    add_param_button.id = "Add_Button";
    add_param_button.type = 'button';
    add_param_button.textContent = "ADD BASE PARAM";
    add_param_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const params = menu['Actor Selection Window']['Draw Base Params'];
        const last_gauge = params[params.length - 1];
        let index = 0;
        if(last_gauge){
            index = eval(last_gauge['ID']) + 1;
        }
        const param_obj = {
            "Name":`New Param: ${index}`,
            "Base Param":"0",
            "Param Text":"",
            "X":"0",
            "Y":"0",
        }
        params.push(param_obj);
        recompileMenus(menu_data, menu, true);
    })
    param_config_container.appendChild(add_param_button);
}

const addActrDataBaseParamForm = function(container, data, index){
    const selection_windows = data['Actor Data Windows'];
    const selection_window = selection_windows[index];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Draw Base Params";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Param_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const param_config_container = document.createElement('div');
    param_config_container.id = "Param_Config_Container";
    param_config_container.classList.add("content");
    container.appendChild(param_config_container);
    const params = selection_window['Draw Base Params'] || [];
    params.forEach((param_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${param_data['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Param_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        param_config_container.appendChild(collapse_button);
        const param_div = document.createElement('div');
        param_div.id = "Param_Container";
        param_div.classList.add("content");
        param_config_container.appendChild(param_div);
        const param_name_label = document.createElement('label');
        param_name_label.id = "Window_Label";
        param_name_label.textContent = `Name: ${param_data['Name']}`;
        param_div.appendChild(param_name_label);
        const param_name_input = document.createElement('input');
        param_name_input.setAttribute("value", param_data['Name']);
        param_name_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_name_input.value){
                    const new_label = param_name_input.value;
                    menu['Actor Data Windows'][index]['Draw Base Params'][sav_index]['Name'] = new_label;
                    param_name_label.textContent = `Name: ${new_label}`;
                    param_data['Name'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_name_input);
        const param_id_label = document.createElement('label');
        param_id_label.id = "Window_Label";
        param_id_label.textContent = `Base Param ID: ${param_data['Base Param']}`;
        param_div.appendChild(param_id_label);
        const param_id_input = document.createElement('input');
        param_id_input.setAttribute("value", param_data['Base Param']);
        param_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_id_input.value){
                    let value = param_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Draw Base Params'][sav_index]['Base Param'] = value.toString();
                    param_name_label.textContent = `Base Param ID: ${value}`;
                    param_data['Base Param'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_id_input);
        const param_text_label = document.createElement('label');
        param_text_label.id = "Window_Label";
        param_text_label.textContent = "Param Text: %1 = Value";
        param_div.appendChild(param_text_label);
        const param_text_input = document.createElement('input');
        param_text_input.id = "Window_Input";
        param_text_input.setAttribute("value", param_data['Param Text']);
        param_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_text_input.value){
                    let value = param_text_input.value;
                    menu['Actor Data Windows'][index]['Draw Base Params'][sav_index]['Param Text'] = value.toString();
                    param_name_label.textContent = `Param Text: %1 = Value`;
                    param_data['Param Text'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_text_input)
        const param_x_label = document.createElement(`label`);
        param_x_label.id = "Window_Label";
        param_x_label.textContent = `X: ${param_data['X']}`;
        param_div.appendChild(param_x_label);
        const param_x_input = document.createElement("input");
        param_x_input.id = "Window_Input";
        param_x_input.setAttribute("value", param_data['X']);
        param_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_x_input.value){
                    let value = param_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Draw Base Params'][sav_index]['X'] = value;
                    param_x_label.textContent = `X: ${value}`;
                    param_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_x_input);
        const param_y_label = document.createElement(`label`);
        param_y_label.id = "Window_Label";
        param_y_label.textContent = `Y: ${param_data['Y']}`;
        param_div.appendChild(param_y_label);
        const param_y_input = document.createElement("input");
        param_y_input.id = "Window_Input";
        param_y_input.setAttribute("value", param_data['Y']);
        param_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_y_input.value){
                    let value = param_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Draw Base Params'][sav_index]['Y'] = value;
                    param_y_label.textContent = `Y: ${value}`;
                    param_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_y_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let del_index = -1;
            for(let i = 0; i < selection_window['Draw Base Params'].length; i++){
                const data = selection_window['Draw Base Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Actor Data Windows'][index]['Draw Base Params'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        param_div.appendChild(delete_button);
    })
    const add_param_button = document.createElement('button');
    add_param_button.id = "Add_Button";
    add_param_button.type = 'button';
    add_param_button.textContent = "ADD BASE PARAM";
    add_param_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const params = menu['Actor Data Windows'][index]['Draw Base Params'];
        const last_gauge = params[params.length - 1];
        let index = 0;
        if(last_gauge){
            index = eval(last_gauge['ID']) + 1;
        }
        const param_obj = {
            "Name":`New Param: ${index}`,
            "Base Param":"0",
            "Param Text":"",
            "X":"0",
            "Y":"0",
        }
        params.push(param_obj);
        recompileMenus(menu_data, menu, true);
    })
    param_config_container.appendChild(add_param_button);
}

const addActrSelcExParamForm = function(container, data){
    const selection_window = data['Actor Selection Window'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Draw Ex Params";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Param_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const param_config_container = document.createElement('div');
    param_config_container.id = "Param_Config_Container";
    param_config_container.classList.add("content");
    container.appendChild(param_config_container);
    const params = selection_window['Draw Ex Params'] || [];
    params.forEach((param_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${param_data['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Param_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        param_config_container.appendChild(collapse_button);
        const param_div = document.createElement('div');
        param_div.id = "Param_Container";
        param_div.classList.add("content");
        param_config_container.appendChild(param_div);
        const param_name_label = document.createElement('label');
        param_name_label.id = "Window_Label";
        param_name_label.textContent = `Name: ${param_data['Name']}`;
        param_div.appendChild(param_name_label);
        const param_name_input = document.createElement('input');
        param_name_input.setAttribute("value", param_data['Name']);
        param_name_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_name_input.value){
                    const new_label = param_name_input.value;
                    menu['Actor Selection Window']['Draw Ex Params'][sav_index]['Name'] = new_label;
                    param_name_label.textContent = `Name: ${new_label}`;
                    param_data['Name'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_name_input);
        const param_id_label = document.createElement('label');
        param_id_label.id = "Window_Label";
        param_id_label.textContent = `Ex Param ID: ${param_data['Ex Param']}`;
        param_div.appendChild(param_id_label);
        const param_id_input = document.createElement('input');
        param_id_input.setAttribute("value", param_data['Ex Param']);
        param_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_id_input.value){
                    let value = param_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Draw Ex Params'][sav_index]['Ex Param'] = value.toString();
                    param_name_label.textContent = `Ex Param ID: ${value}`;
                    param_data['Ex Param'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_id_input);
        const param_text_label = document.createElement('label');
        param_text_label.id = "Window_Label";
        param_text_label.textContent = "Param Text: %1 = Value";
        param_div.appendChild(param_text_label);
        const param_text_input = document.createElement('input');
        param_text_input.id = "Window_Input";
        param_text_input.setAttribute("value", param_data['Param Text']);
        param_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_text_input.value){
                    let value = param_text_input.value;
                    menu['Actor Selection Window']['Draw Ex Params'][sav_index]['Param Text'] = value.toString();
                    param_name_label.textContent = `Param Text: %1 = Value`;
                    param_data['Param Text'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_text_input)
        const param_x_label = document.createElement(`label`);
        param_x_label.id = "Window_Label";
        param_x_label.textContent = `X: ${param_data['X']}`;
        param_div.appendChild(param_x_label);
        const param_x_input = document.createElement("input");
        param_x_input.id = "Window_Input";
        param_x_input.setAttribute("value", param_data['X']);
        param_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_x_input.value){
                    let value = param_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Draw Ex Params'][sav_index]['X'] = value;
                    param_x_label.textContent = `X: ${value}`;
                    param_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_x_input);
        const param_y_label = document.createElement(`label`);
        param_y_label.id = "Window_Label";
        param_y_label.textContent = `Y: ${param_data['Y']}`;
        param_div.appendChild(param_y_label);
        const param_y_input = document.createElement("input");
        param_y_input.id = "Window_Input";
        param_y_input.setAttribute("value", param_data['Y']);
        param_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_y_input.value){
                    let value = param_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Draw Ex Params'][sav_index]['Y'] = value;
                    param_y_label.textContent = `Y: ${value}`;
                    param_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_y_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let del_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Actor Selection Window']['Draw Ex Params'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        param_div.appendChild(delete_button);
    })
    const add_param_button = document.createElement('button');
    add_param_button.id = "Add_Button";
    add_param_button.type = 'button';
    add_param_button.textContent = "ADD EX PARAM";
    add_param_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const params = menu['Actor Selection Window']['Draw Ex Params'];
        let index = params.length;
        const param_obj = {
            "Name":`New Param: ${index}`,
            "Ex Param":"0",
            "Param Text":"",
            "X":"0",
            "Y":"0",
        }
        params.push(param_obj);
        recompileMenus(menu_data, menu, true);
    })
    param_config_container.appendChild(add_param_button);
}

const addActrDataExParamForm = function(container, data, index){
    const selection_windows = data['Actor Data Windows'];
    const selection_window = selection_windows[index];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Draw Ex Params";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Param_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const param_config_container = document.createElement('div');
    param_config_container.id = "Param_Config_Container";
    param_config_container.classList.add("content");
    container.appendChild(param_config_container);
    const params = selection_window['Draw Ex Params'] || [];
    params.forEach((param_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${param_data['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Param_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        param_config_container.appendChild(collapse_button);
        const param_div = document.createElement('div');
        param_div.id = "Param_Container";
        param_div.classList.add("content");
        param_config_container.appendChild(param_div);
        const param_name_label = document.createElement('label');
        param_name_label.id = "Window_Label";
        param_name_label.textContent = `Name: ${param_data['Name']}`;
        param_div.appendChild(param_name_label);
        const param_name_input = document.createElement('input');
        param_name_input.setAttribute("value", param_data['Name']);
        param_name_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_name_input.value){
                    const new_label = param_name_input.value;
                    menu['Actor Data Windows'][index]['Draw Ex Params'][sav_index]['Name'] = new_label;
                    param_name_label.textContent = `Name: ${new_label}`;
                    param_data['Name'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_name_input);
        const param_id_label = document.createElement('label');
        param_id_label.id = "Window_Label";
        param_id_label.textContent = `Ex Param ID: ${param_data['Ex Param']}`;
        param_div.appendChild(param_id_label);
        const param_id_input = document.createElement('input');
        param_id_input.setAttribute("value", param_data['Ex Param']);
        param_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_id_input.value){
                    let value = param_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Draw Ex Params'][sav_index]['Ex Param'] = value.toString();
                    param_name_label.textContent = `Ex Param ID: ${value}`;
                    param_data['Ex Param'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_id_input);
        const param_text_label = document.createElement('label');
        param_text_label.id = "Window_Label";
        param_text_label.textContent = "Param Text: %1 = Value";
        param_div.appendChild(param_text_label);
        const param_text_input = document.createElement('input');
        param_text_input.id = "Window_Input";
        param_text_input.setAttribute("value", param_data['Param Text']);
        param_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_text_input.value){
                    let value = param_text_input.value;
                    menu['Actor Data Windows'][index]['Draw Ex Params'][sav_index]['Param Text'] = value.toString();
                    param_name_label.textContent = `Param Text: %1 = Value`;
                    param_data['Param Text'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_text_input)
        const param_x_label = document.createElement(`label`);
        param_x_label.id = "Window_Label";
        param_x_label.textContent = `X: ${param_data['X']}`;
        param_div.appendChild(param_x_label);
        const param_x_input = document.createElement("input");
        param_x_input.id = "Window_Input";
        param_x_input.setAttribute("value", param_data['X']);
        param_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_x_input.value){
                    let value = param_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Draw Ex Params'][sav_index]['X'] = value;
                    param_x_label.textContent = `X: ${value}`;
                    param_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_x_input);
        const param_y_label = document.createElement(`label`);
        param_y_label.id = "Window_Label";
        param_y_label.textContent = `Y: ${param_data['Y']}`;
        param_div.appendChild(param_y_label);
        const param_y_input = document.createElement("input");
        param_y_input.id = "Window_Input";
        param_y_input.setAttribute("value", param_data['Y']);
        param_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_y_input.value){
                    let value = param_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Draw Ex Params'][sav_index]['Y'] = value;
                    param_y_label.textContent = `Y: ${value}`;
                    param_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_y_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let del_index = -1;
            for(let i = 0; i < selection_window['Draw Ex Params'].length; i++){
                const data = selection_window['Draw Ex Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Actor Data Windows'][index]['Draw Ex Params'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        param_div.appendChild(delete_button);
    })
    const add_param_button = document.createElement('button');
    add_param_button.id = "Add_Button";
    add_param_button.type = 'button';
    add_param_button.textContent = "ADD EX PARAM";
    add_param_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const params = menu['Actor Data Windows'][index]['Draw Ex Params'];
        let index = params.length;
        const param_obj = {
            "Name":`New Param: ${index}`,
            "Ex Param":"0",
            "Param Text":"",
            "X":"0",
            "Y":"0",
        }
        params.push(param_obj);
        recompileMenus(menu_data, menu, true);
    })
    param_config_container.appendChild(add_param_button);
}

const addActrSelcSpParamForm = function(container, data){
    const selection_window = data['Actor Selection Window'];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Draw Sp Params";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Param_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const param_config_container = document.createElement('div');
    param_config_container.id = "Param_Config_Container";
    param_config_container.classList.add("content");
    container.appendChild(param_config_container);
    const params = selection_window['Draw Sp Params'] || [];
    params.forEach((param_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${param_data['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Param_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        param_config_container.appendChild(collapse_button);
        const param_div = document.createElement('div');
        param_div.id = "Param_Container";
        param_div.classList.add("content");
        param_config_container.appendChild(param_div);
        const param_name_label = document.createElement('label');
        param_name_label.id = "Window_Label";
        param_name_label.textContent = `Name: ${param_data['Name']}`;
        param_div.appendChild(param_name_label);
        const param_name_input = document.createElement('input');
        param_name_input.setAttribute("value", param_data['Name']);
        param_name_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_name_input.value){
                    const new_label = param_name_input.value;
                    menu['Actor Selection Window']['Draw Sp Params'][sav_index]['Name'] = new_label;
                    param_name_label.textContent = `Name: ${new_label}`;
                    param_data['Name'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_name_input);
        const param_id_label = document.createElement('label');
        param_id_label.id = "Window_Label";
        param_id_label.textContent = `Sp Param ID: ${param_data['Sp Param']}`;
        param_div.appendChild(param_id_label);
        const param_id_input = document.createElement('input');
        param_id_input.setAttribute("value", param_data['Sp Param']);
        param_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_id_input.value){
                    let value = param_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Draw Sp Params'][sav_index]['Sp Param'] = value.toString();
                    param_name_label.textContent = `Sp Param ID: ${value}`;
                    param_data['Sp Param'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_id_input);
        const param_text_label = document.createElement('label');
        param_text_label.id = "Window_Label";
        param_text_label.textContent = "Param Text: %1 = Value";
        param_div.appendChild(param_text_label);
        const param_text_input = document.createElement('input');
        param_text_input.id = "Window_Input";
        param_text_input.setAttribute("value", param_data['Param Text']);
        param_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_text_input.value){
                    let value = param_text_input.value;
                    menu['Actor Selection Window']['Draw Sp Params'][sav_index]['Param Text'] = value.toString();
                    param_name_label.textContent = `Param Text: %1 = Value`;
                    param_data['Param Text'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_text_input)
        const param_x_label = document.createElement(`label`);
        param_x_label.id = "Window_Label";
        param_x_label.textContent = `X: ${param_data['X']}`;
        param_div.appendChild(param_x_label);
        const param_x_input = document.createElement("input");
        param_x_input.id = "Window_Input";
        param_x_input.setAttribute("value", param_data['X']);
        param_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_x_input.value){
                    let value = param_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Draw Sp Params'][sav_index]['X'] = value;
                    param_x_label.textContent = `X: ${value}`;
                    param_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_x_input);
        const param_y_label = document.createElement(`label`);
        param_y_label.id = "Window_Label";
        param_y_label.textContent = `Y: ${param_data['Y']}`;
        param_div.appendChild(param_y_label);
        const param_y_input = document.createElement("input");
        param_y_input.id = "Window_Input";
        param_y_input.setAttribute("value", param_data['Y']);
        param_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_y_input.value){
                    let value = param_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Selection Window']['Draw Sp Params'][sav_index]['Y'] = value;
                    param_y_label.textContent = `Y: ${value}`;
                    param_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_y_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Selection Window'];
            let del_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Actor Selection Window']['Draw Sp Params'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        param_div.appendChild(delete_button);
    })
    const add_param_button = document.createElement('button');
    add_param_button.id = "Add_Button";
    add_param_button.type = 'button';
    add_param_button.textContent = "ADD SP PARAM";
    add_param_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const params = menu['Actor Selection Window']['Draw Sp Params'];
        let index = params.length;
        const param_obj = {
            "Name":`New Param: ${index}`,
            "Sp Param":"0",
            "Param Text":"",
            "X":"0",
            "Y":"0",
        }
        params.push(param_obj);
        recompileMenus(menu_data, menu, true);
    })
    param_config_container.appendChild(add_param_button);
}

const addActrDataSpParamForm = function(container, data, index){
    const selection_windows = data['Actor Data Windows'];
    const selection_window = selection_windows[index];
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button_1';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Draw Sp Params";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Param_Config_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    container.appendChild(collapse_button);
    const param_config_container = document.createElement('div');
    param_config_container.id = "Param_Config_Container";
    param_config_container.classList.add("content");
    container.appendChild(param_config_container);
    const params = selection_window['Draw Sp Params'] || [];
    params.forEach((param_data)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button_2';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${param_data['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Param_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        param_config_container.appendChild(collapse_button);
        const param_div = document.createElement('div');
        param_div.id = "Param_Container";
        param_div.classList.add("content");
        param_config_container.appendChild(param_div);
        const param_name_label = document.createElement('label');
        param_name_label.id = "Window_Label";
        param_name_label.textContent = `Name: ${param_data['Name']}`;
        param_div.appendChild(param_name_label);
        const param_name_input = document.createElement('input');
        param_name_input.setAttribute("value", param_data['Name']);
        param_name_input.addEventListener("input", (event)=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_name_input.value){
                    const new_label = param_name_input.value;
                    menu['Actor Data Windows'][index]['Draw Sp Params'][sav_index]['Name'] = new_label;
                    param_name_label.textContent = `Name: ${new_label}`;
                    param_data['Name'] = new_label;
                    collapse_button.textContent = new_label;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_name_input);
        const param_id_label = document.createElement('label');
        param_id_label.id = "Window_Label";
        param_id_label.textContent = `Sp Param ID: ${param_data['Sp Param']}`;
        param_div.appendChild(param_id_label);
        const param_id_input = document.createElement('input');
        param_id_input.setAttribute("value", param_data['Sp Param']);
        param_id_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_id_input.value){
                    let value = param_id_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Draw Sp Params'][sav_index]['Sp Param'] = value.toString();
                    param_name_label.textContent = `Sp Param ID: ${value}`;
                    param_data['Sp Param'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_id_input);
        const param_text_label = document.createElement('label');
        param_text_label.id = "Window_Label";
        param_text_label.textContent = "Param Text: %1 = Value";
        param_div.appendChild(param_text_label);
        const param_text_input = document.createElement('input');
        param_text_input.id = "Window_Input";
        param_text_input.setAttribute("value", param_data['Param Text']);
        param_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_text_input.value){
                    let value = param_text_input.value;
                    menu['Actor Data Windows'][index]['Draw Sp Params'][sav_index]['Param Text'] = value.toString();
                    param_name_label.textContent = `Param Text: %1 = Value`;
                    param_data['Param Text'] = value.toString();
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_text_input)
        const param_x_label = document.createElement(`label`);
        param_x_label.id = "Window_Label";
        param_x_label.textContent = `X: ${param_data['X']}`;
        param_div.appendChild(param_x_label);
        const param_x_input = document.createElement("input");
        param_x_input.id = "Window_Input";
        param_x_input.setAttribute("value", param_data['X']);
        param_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_x_input.value){
                    let value = param_x_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Draw Sp Params'][sav_index]['X'] = value;
                    param_x_label.textContent = `X: ${value}`;
                    param_data['X'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_x_input);
        const param_y_label = document.createElement(`label`);
        param_y_label.id = "Window_Label";
        param_y_label.textContent = `Y: ${param_data['Y']}`;
        param_div.appendChild(param_y_label);
        const param_y_input = document.createElement("input");
        param_y_input.id = "Window_Input";
        param_y_input.setAttribute("value", param_data['Y']);
        param_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let sav_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        sav_index = i;
                        break;
                    }
                }
            }
            if(sav_index >= 0){
                if(param_y_input.value){
                    let value = param_y_input.value;
                    if(isNaN(eval(value)))value = 0;
                    menu['Actor Data Windows'][index]['Draw Sp Params'][sav_index]['Y'] = value;
                    param_y_label.textContent = `Y: ${value}`;
                    param_data['Y'] = value;
                    recompileMenus(menu_data, menu);
                }
            }
        })
        param_div.appendChild(param_y_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const selection_window = menu['Actor Data Windows'][index];
            let del_index = -1;
            for(let i = 0; i < selection_window['Draw Sp Params'].length; i++){
                const data = selection_window['Draw Sp Params'][i];
                if(data){
                    if(data['Name'] == param_data['Name']){
                        del_index = i;
                        break;
                    }
                }
            }
            if(del_index >= 0){
                menu['Actor Data Windows'][index]['Draw Sp Params'].splice(del_index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        param_div.appendChild(delete_button);
    })
    const add_param_button = document.createElement('button');
    add_param_button.id = "Add_Button";
    add_param_button.type = 'button';
    add_param_button.textContent = "ADD SP PARAM";
    add_param_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const params = menu['Actor Data Windows'][index]['Draw Sp Params'];
        let index = params.length;
        const param_obj = {
            "Name":`New Param: ${index}`,
            "Sp Param":"0",
            "Param Text":"",
            "X":"0",
            "Y":"0",
        }
        params.push(param_obj);
        recompileMenus(menu_data, menu, true);
    })
    param_config_container.appendChild(add_param_button);
}

const dirToString = function(dir){
    dir = eval(dir);
    switch(dir){
        case 1: return `down left`;
        case 2: return `down`;
        case 3: return `down right`;
        case 4: return `left`;
        case 6: return `right`;
        case 7: return `up left`;
        case 8: return `up`;
        case 9: return `up right`;
    }
    return '???';
}

const createActrSelcWindowForm = function(form, data){
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Actor Selection Window";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Actor_Selection_Window_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const selec_window = data['Actor Selection Window'];
    const selc_window_container = document.createElement('div');
    selc_window_container.id = "Actor_Selection_Window_Container";
    selc_window_container.classList.add("content");
    addActrSelcWindowDimensionForm(selc_window_container, data);
    addActrSelcWindowStyleForm(selc_window_container, data);
    const itm_width_label = document.createElement('label');
    itm_width_label.id = "Window_Label";
    itm_width_label.textContent = `Item Width: ${selec_window['Item Width']}`;
    selc_window_container.appendChild(itm_width_label);
    const itm_width_input = document.createElement("input");
    itm_width_input.id = "Window_Input";
    itm_width_input.setAttribute("type", "text");
    itm_width_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = itm_width_input.value;
            if(isNaN(eval(value)))value = 0;
            itm_width_label.textContent = `Item Width: ${value}`;
            selection_window['Item Width'] = value;
            data['Item Width'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(itm_width_input);
    const itm_height_label = document.createElement('label');
    itm_height_label.id = "Window_Label";
    itm_height_label.textContent = `Item Height: ${selec_window['Item Height']}`;
    selc_window_container.appendChild(itm_height_label);
    const itm_height_input = document.createElement("input");
    itm_height_input.id = "Window_Input";
    itm_height_input.setAttribute("type", "text");
    itm_height_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = itm_height_input.value;
            if(isNaN(eval(value)))value = 0;
            itm_height_label.textContent = `Item Height: ${value}`;
            selection_window['Item Height'] = value;
            data['Item Height'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(itm_height_input);
    const max_cols_input = document.createElement("input");
    max_cols_input.id = "Window_Input";
    max_cols_input.setAttribute("type", "text");
    max_cols_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = max_cols_input.value;
            if(isNaN(eval(value)))value = 0;
            max_cols_label.textContent = `Max Columns: ${value}`;
            selection_window['Max Columns'] = value;
            data['Max Columns'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(max_cols_input);
    addActrSelcWindowGaugesForm(selc_window_container, data);
    const drw_actr_name_label = document.createElement('label');
    drw_actr_name_label.id = "Window_Label";
    drw_actr_name_label.textContent = `Draw Actor Name: ${eval(selec_window['Draw Actor Name']) ? "true" : "false"}`;
    selc_window_container.appendChild(drw_actr_name_label);
    const drw_actr_name_input = document.createElement("input");
    drw_actr_name_input.id = "Window_Input";
    drw_actr_name_input.setAttribute("type", "checkbox");
    drw_actr_name_input.setAttribute("checked", eval(selec_window['Draw Actor Name']));
    drw_actr_name_input.checked = eval(selec_window['Draw Actor Name']);
    drw_actr_name_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_actr_name_input.checked;
            drw_actr_name_label.textContent = `Draw Actor Name: ${value}`;
            selection_window['Draw Actor Name'] = value.toString();
            data['Draw Actor Name'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_actr_name_input);
    const drw_name_text_label = document.createElement('label');
    drw_name_text_label.id = "Window_Label";
    drw_name_text_label.textContent = `Name Text: %1 = Actor Name`;
    selc_window_container.appendChild(drw_name_text_label);
    const drw_name_text_input = document.createElement("input");
    drw_name_text_input.id = "Window_Input";
    drw_name_text_input.setAttribute("type", "text");
    drw_name_text_input.setAttribute("value", selec_window['Name Text']);
    drw_name_text_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_name_text_input.value;
            drw_name_text_label.textContent = `Name Text: %1 = Actor Name`;
            selection_window['Name Text'] = value;
            data['Name Text'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_name_text_input);
    const drw_name_x_label = document.createElement('label');
    drw_name_x_label.id = "Window_Label";
    drw_name_x_label.textContent = `Name X: ${selec_window['Name X']}`;
    selc_window_container.appendChild(drw_name_x_label);
    const drw_name_x_input = document.createElement("input");
    drw_name_x_input.id = "Window_Input";
    drw_name_x_input.setAttribute("type", "text");
    drw_name_x_input.setAttribute("value", selec_window['Name X']);
    drw_name_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_name_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_name_x_label.textContent = `Name X: ${value}`;
            selection_window['Name X'] = value;
            data['Name X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_name_x_input);
    const drw_name_y_label = document.createElement('label');
    drw_name_y_label.id = "Window_Label";
    drw_name_y_label.textContent = `Name Y: ${selec_window['Name Y']}`;
    selc_window_container.appendChild(drw_name_y_label);
    const drw_name_y_input = document.createElement("input");
    drw_name_y_input.id = "Window_Input";
    drw_name_y_input.setAttribute("type", "text");
    drw_name_y_input.setAttribute("value", selec_window['Name Y']);
    drw_name_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_name_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_name_y_label.textContent = `Name Y: ${value}`;
            selection_window['Name Y'] = value;
            data['Name Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_name_y_input);
    const drw_actr_profile_label = document.createElement('label');
    drw_actr_profile_label.id = "Window_Label";
    drw_actr_profile_label.textContent = `Draw Actor Profile: ${eval(selec_window['Draw Actor Profile']) ? "true" : "false"}`;
    selc_window_container.appendChild(drw_actr_profile_label);
    const drw_actr_profile_input = document.createElement("input");
    drw_actr_profile_input.id = "Window_Input";
    drw_actr_profile_input.setAttribute("type", "checkbox");
    drw_actr_profile_input.setAttribute("checked", eval(selec_window['Draw Actor Profile']));
    drw_actr_profile_input.checked = eval(selec_window['Draw Actor Profile']);
    drw_actr_profile_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_actr_profile_input.checked;
            drw_actr_profile_label.textContent = `Draw Actor Profile: ${value}`;
            selection_window['Draw Actor Name'] = value.toString();
            data['Draw Actor Name'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_actr_profile_input);
    const drw_profile_x_label = document.createElement('label');
    drw_profile_x_label.id = "Window_Label";
    drw_profile_x_label.textContent = `Profile X: ${selec_window['Profile X']}`;
    selc_window_container.appendChild(drw_profile_x_label);
    const drw_profile_x_input = document.createElement("input");
    drw_profile_x_input.id = "Window_Input";
    drw_profile_x_input.setAttribute("type", "text");
    drw_profile_x_input.setAttribute("value", selec_window['Profile X']);
    drw_profile_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_profile_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_profile_x_label.textContent = `Profile X: ${value}`;
            selection_window['Profile X'] = value.toString();
            data['Profile X'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_profile_x_input);
    const drw_profile_y_label = document.createElement('label');
    drw_profile_y_label.id = "Window_Label";
    drw_profile_y_label.textContent = `Profile Y: ${selec_window['Profile Y']}`;
    selc_window_container.appendChild(drw_profile_y_label);
    const drw_profile_y_input = document.createElement("input");
    drw_profile_y_input.id = "Window_Input";
    drw_profile_y_input.setAttribute("type", "text");
    drw_profile_y_input.setAttribute("value", selec_window['Profile Y']);
    drw_profile_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_profile_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_profile_y_label.textContent = `Profile Y: ${value}`;
            selection_window['Profile Y'] = value;
            data['Profile Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_profile_y_input);
    const drw_cls_lvl_label = document.createElement('label');
    drw_cls_lvl_label.id = "Window_Label";
    drw_cls_lvl_label.textContent = `Draw Class Level: ${eval(selec_window['Draw Class Level']) ? "true" : "false"}`;
    selc_window_container.appendChild(drw_cls_lvl_label);
    const drw_cls_lvl_input = document.createElement("input");
    drw_cls_lvl_input.id = "Window_Input";
    drw_cls_lvl_input.setAttribute("type", "checkbox");
    drw_cls_lvl_input.setAttribute("checked", eval(selec_window['Draw Class Level']));
    drw_cls_lvl_input.checked = eval(selec_window['Draw Class Level']);
    drw_cls_lvl_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_cls_lvl_input.checked;
            drw_cls_lvl_label.textContent = `Draw Class Level: ${value}`;
            selection_window['Draw Class Level'] = value.toString();
            data['Draw Class Level'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_cls_lvl_input);
    const drw_cls_lvl_text_label = document.createElement('label');
    drw_cls_lvl_text_label.id = "Window_Label";
    drw_cls_lvl_text_label.textContent = `Class Level Text: %1 = Class Name`;
    selc_window_container.appendChild(drw_cls_lvl_text_label);
    const drw_cls_lvl_text_input = document.createElement("input");
    drw_cls_lvl_text_input.id = "Window_Input";
    drw_cls_lvl_text_input.setAttribute("type", "text");
    drw_cls_lvl_text_input.setAttribute("value", selec_window['Class Level Text']);
    drw_cls_lvl_text_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_cls_lvl_text_input.value;
            drw_cls_lvl_text_label.textContent = `Class Level Text: %1 = Class Name`;
            selection_window['Class Level Text'] = value;
            data['Class Level Text'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_cls_lvl_text_input);
    const drw_cls_lvl_x_label = document.createElement('label');
    drw_cls_lvl_x_label.id = "Window_Label";
    drw_cls_lvl_x_label.textContent = `Class Level X: ${selec_window['Class Level X']}`;
    selc_window_container.appendChild(drw_cls_lvl_x_label);
    const drw_cls_lvl_x_input = document.createElement("input");
    drw_cls_lvl_x_input.id = "Window_Input";
    drw_cls_lvl_x_input.setAttribute("type", "text");
    drw_cls_lvl_x_input.setAttribute("value", selec_window['Class Level X']);
    drw_cls_lvl_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_cls_lvl_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_cls_lvl_x_label.textContent = `Class Level X: ${value}`;
            selection_window['Class Level X'] = value;
            data['Class Level X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_cls_lvl_x_input);
    const drw_cls_lvl_y_label = document.createElement('label');
    drw_cls_lvl_y_label.id = "Window_Label";
    drw_cls_lvl_y_label.textContent = `Class Level Y: ${selec_window['Class Level Y']}`;
    selc_window_container.appendChild(drw_cls_lvl_y_label);
    const drw_cls_lvl_y_input = document.createElement("input");
    drw_cls_lvl_y_input.id = "Window_Input";
    drw_cls_lvl_y_input.setAttribute("type", "text");
    drw_cls_lvl_y_input.setAttribute("value", selec_window['Class Level Y']);
    drw_cls_lvl_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_cls_lvl_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_cls_lvl_y_label.textContent = `Class Level Y: ${value}`;
            selection_window['Class Level Y'] = value;
            data['Class Level Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_cls_lvl_y_input);
    const drw_hp_res_label = document.createElement('label');
    drw_hp_res_label.id = "Window_Label";
    drw_hp_res_label.textContent = `Draw HP Resource: ${eval(selec_window['Draw HP Resource']) ? "true" : "false"}`;
    selc_window_container.appendChild(drw_hp_res_label);
    const drw_hp_res_input = document.createElement("input");
    drw_hp_res_input.id = "Window_Input";
    drw_hp_res_input.setAttribute("type", "checkbox");
    drw_hp_res_input.setAttribute("checked", eval(selec_window['Draw HP Resource']));
    drw_hp_res_input.checked = eval(selec_window['Draw HP Resource']);
    drw_hp_res_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_hp_res_input.checked;
            drw_hp_res_label.textContent = `Draw HP Resource: ${value}`;
            selection_window['Draw HP Resource'] = value.toString();
            data['Draw HP Resource'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_hp_res_input);
    const drw_hp_res_text_label = document.createElement('label');
    drw_hp_res_text_label.id = "Window_Label";
    drw_hp_res_text_label.textContent = `HP Text: %1 = Current, %2 = Max`;
    selc_window_container.appendChild(drw_hp_res_text_label);
    const drw_hp_res_text_input = document.createElement("input");
    drw_hp_res_text_input.id = "Window_Input";
    drw_hp_res_text_input.setAttribute("type", "text");
    drw_hp_res_text_input.setAttribute("value", selec_window['HP Text']);
    drw_hp_res_text_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_hp_res_text_input.value;
            drw_hp_res_text_label.textContent = `HP Text: %1 = Current, %2 = Max`;
            selection_window['HP Text'] = value;
            data['HP Text'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_hp_res_text_input);
    const drw_hp_res_x_label = document.createElement('label');
    drw_hp_res_x_label.id = "Window_Label";
    drw_hp_res_x_label.textContent = `HP X: ${selec_window['HP X']}`;
    selc_window_container.appendChild(drw_hp_res_x_label);
    const drw_hp_res_x_input = document.createElement("input");
    drw_hp_res_x_input.id = "Window_Input";
    drw_hp_res_x_input.setAttribute("type", "text");
    drw_hp_res_x_input.setAttribute("value", selec_window['HP X']);
    drw_hp_res_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_hp_res_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_hp_res_x_label.textContent = `HP X: ${value}`;
            selection_window['HP X'] = value;
            data['HP X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_hp_res_x_input);
    const drw_hp_res_y_label = document.createElement('label');
    drw_hp_res_y_label.id = "Window_Label";
    drw_hp_res_y_label.textContent = `HP Y: ${selec_window['HP Y']}`;
    selc_window_container.appendChild(drw_hp_res_y_label);
    const drw_hp_res_y_input = document.createElement("input");
    drw_hp_res_y_input.id = "Window_Input";
    drw_hp_res_y_input.setAttribute("type", "text");
    drw_hp_res_y_input.setAttribute("value", selec_window['HP Y']);
    drw_hp_res_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_hp_res_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_hp_res_y_label.textContent = `HP Y: ${value}`;
            selection_window['HP Y'] = value;
            data['HP Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_hp_res_y_input);
    const drw_mp_res_label = document.createElement('label');
    drw_mp_res_label.id = "Window_Label";
    drw_mp_res_label.textContent = `Draw MP Resource: ${eval(selec_window['Draw MP Resource']) ? "true" : "false"}`;
    selc_window_container.appendChild(drw_mp_res_label);
    const drw_mp_res_input = document.createElement("input");
    drw_mp_res_input.id = "Window_Input";
    drw_mp_res_input.setAttribute("type", "checkbox");
    drw_mp_res_input.setAttribute("checked", eval(selec_window['Draw MP Resource']));
    drw_mp_res_input.checked = eval(selec_window['Draw MP Resource']);
    drw_mp_res_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_mp_res_input.checked;
            drw_mp_res_label.textContent = `Draw MP Resource: ${value}`;
            selection_window['Draw MP Resource'] = value.toString();
            data['Draw MP Resource'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_mp_res_input);
    const drw_mp_res_text_label = document.createElement('label');
    drw_mp_res_text_label.id = "Window_Label";
    drw_mp_res_text_label.textContent = `MP Text: %1 = Current, %2 = Max`;
    selc_window_container.appendChild(drw_mp_res_text_label);
    const drw_mp_res_text_input = document.createElement("input");
    drw_mp_res_text_input.id = "Window_Input";
    drw_mp_res_text_input.setAttribute("type", "text");
    drw_mp_res_text_input.setAttribute("value", selec_window['MP Text']);
    drw_mp_res_text_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_mp_res_text_input.value;
            drw_mp_res_text_label.textContent = `MP Text: %1 = Current, %2 = Max`;
            selection_window['MP Text'] = value;
            data['MP Text'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_mp_res_text_input);
    const drw_mp_res_x_label = document.createElement('label');
    drw_mp_res_x_label.id = "Window_Label";
    drw_mp_res_x_label.textContent = `MP X: ${selec_window['MP X']}`;
    selc_window_container.appendChild(drw_mp_res_x_label);
    const drw_mp_res_x_input = document.createElement("input");
    drw_mp_res_x_input.id = "Window_Input";
    drw_mp_res_x_input.setAttribute("type", "text");
    drw_mp_res_x_input.setAttribute("value", selec_window['MP X']);
    drw_mp_res_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_mp_res_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_mp_res_x_label.textContent = `MP X: ${value}`;
            selection_window['MP X'] = value;
            data['MP X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_mp_res_x_input);
    const drw_mp_res_y_label = document.createElement('label');
    drw_mp_res_y_label.id = "Window_Label";
    drw_mp_res_y_label.textContent = `MP Y: ${selec_window['MP Y']}`;
    selc_window_container.appendChild(drw_mp_res_y_label);
    const drw_mp_res_y_input = document.createElement("input");
    drw_mp_res_y_input.id = "Window_Input";
    drw_mp_res_y_input.setAttribute("type", "text");
    drw_mp_res_y_input.setAttribute("value", selec_window['MP Y']);
    drw_mp_res_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_mp_res_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_mp_res_y_label.textContent = `MP Y: ${value}`;
            selection_window['MP Y'] = value;
            data['MP Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_mp_res_y_input);
    const drw_tp_res_label = document.createElement('label');
    drw_tp_res_label.id = "Window_Label";
    drw_tp_res_label.textContent = `Draw TP Resource: ${eval(selec_window['Draw TP Resource']) ? "true" : "false"}`;
    selc_window_container.appendChild(drw_tp_res_label);
    const drw_tp_res_input = document.createElement("input");
    drw_tp_res_input.id = "Window_Input";
    drw_tp_res_input.setAttribute("type", "checkbox");
    drw_tp_res_input.setAttribute("checked", eval(selec_window['Draw TP Resource']));
    drw_tp_res_input.checked = eval(selec_window['Draw TP Resource']);
    drw_tp_res_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_tp_res_input.checked;
            drw_tp_res_label.textContent = `Draw TP Resource: ${value}`;
            selection_window['Draw TP Resource'] = value.toString();
            data['Draw TP Resource'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_tp_res_input);
    const drw_tp_res_text_label = document.createElement('label');
    drw_tp_res_text_label.id = "Window_Label";
    drw_tp_res_text_label.textContent = `TP Text: %1 = Current, %2 = Max`;
    selc_window_container.appendChild(drw_tp_res_text_label);
    const drw_tp_res_text_input = document.createElement("input");
    drw_tp_res_text_input.id = "Window_Input";
    drw_tp_res_text_input.setAttribute("type", "text");
    drw_tp_res_text_input.setAttribute("value", selec_window['TP Text']);
    drw_tp_res_text_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_tp_res_text_input.value;
            drw_tp_res_text_label.textContent = `TP Text: %1 = Current, %2 = Max`;
            selection_window['TP Text'] = value;
            data['TP Text'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_tp_res_text_input);
    const drw_tp_res_x_label = document.createElement('label');
    drw_tp_res_x_label.id = "Window_Label";
    drw_tp_res_x_label.textContent = `TP X: ${selec_window['TP X']}`;
    selc_window_container.appendChild(drw_tp_res_x_label);
    const drw_tp_res_x_input = document.createElement("input");
    drw_tp_res_x_input.id = "Window_Input";
    drw_tp_res_x_input.setAttribute("type", "text");
    drw_tp_res_x_input.setAttribute("value", selec_window['TP X']);
    drw_tp_res_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_tp_res_x_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_tp_res_x_label.textContent = `TP X: ${value}`;
            selection_window['TP X'] = value;
            data['TP X'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_tp_res_x_input);
    const drw_tp_res_y_label = document.createElement('label');
    drw_tp_res_y_label.id = "Window_Label";
    drw_tp_res_y_label.textContent = `TP Y: ${selec_window['TP Y']}`;
    selc_window_container.appendChild(drw_tp_res_y_label);
    const drw_tp_res_y_input = document.createElement("input");
    drw_tp_res_y_input.id = "Window_Input";
    drw_tp_res_y_input.setAttribute("type", "text");
    drw_tp_res_y_input.setAttribute("value", selec_window['TP Y']);
    drw_tp_res_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = drw_tp_res_y_input.value;
            if(isNaN(eval(value)))value = 0;
            drw_tp_res_y_label.textContent = `TP Y: ${value}`;
            selection_window['TP Y'] = value;
            data['TP Y'] = value;
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(drw_tp_res_y_input);
    addActrSelcBaseParamForm(selc_window_container, data);
    addActrSelcExParamForm(selc_window_container, data);
    addActrSelcSpParamForm(selc_window_container, data);
    const disp_map_char_label = document.createElement('label');
    disp_map_char_label.id = "Window_Label";
    disp_map_char_label.textContent = `Display Map Character: ${eval(selec_window['Display Map Character'])}`;
    selc_window_container.appendChild(disp_map_char_label);
    const disp_map_char_input = document.createElement("input");
    disp_map_char_input.id = "Window_Input";
    disp_map_char_input.setAttribute("type", "checkbox");
    disp_map_char_input.setAttribute("checked", eval(selec_window['Display Map Character']));
    disp_map_char_input.checked = eval(selec_window['Display Map Character']);
    disp_map_char_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_map_char_input.checked;
            disp_map_char_label.textContent = `Display Map Character: ${value}`;
            selection_window['Display Map Character'] = value.toString();
            data['Display Map Character'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(disp_map_char_input);
    const disp_map_char_dir_label = document.createElement("label");
    disp_map_char_dir_label.id = "Window_Label";
    disp_map_char_dir_label.textContent = `Character Direction: ${dirToString(selec_window['Character Direction'])}`;
    selc_window_container.appendChild(disp_map_char_dir_label);
    const disp_map_char_dir_input = document.createElement("input");
    disp_map_char_dir_input.id = "Window_Input";
    disp_map_char_dir_input.setAttribute("value", selec_window['Character Direction']);
    disp_map_char_dir_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_map_char_dir_input.value;
            if(isNaN(eval(value)))value = 0;
            disp_map_char_dir_label.textContent = `Character Direction: ${dirToString(value)}`;
            selection_window['Character Direction'] = value;
            data['Character Direction'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_map_char_dir_input)
    const disp_map_char_x_label = document.createElement("label");
    disp_map_char_x_label.id = "Window_Label";
    disp_map_char_x_label.textContent = `Character X: ${selec_window['Character X']}`;
    selc_window_container.appendChild(disp_map_char_x_label);
    const disp_map_char_x_input = document.createElement("input");
    disp_map_char_x_input.id = "Window_Input";
    disp_map_char_x_input.setAttribute("value", selec_window['Character X']);
    disp_map_char_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_map_char_x_input.value;
            if(isNaN(eval(value)))value = 0;
            disp_map_char_x_label.textContent = `Character X: ${value}`;
            selection_window['Character X'] = value;
            data['Character X'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_map_char_x_input);
    const disp_map_char_y_label = document.createElement("label");
    disp_map_char_y_label.id = "Window_Label";
    disp_map_char_y_label.textContent = `Character Y: ${selec_window['Character Y']}`;
    selc_window_container.appendChild(disp_map_char_y_label);
    const disp_map_char_y_input = document.createElement("input");
    disp_map_char_y_input.id = "Window_Input";
    disp_map_char_y_input.setAttribute("value", selec_window['Character Y']);
    disp_map_char_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_map_char_y_input.value;
            if(isNaN(eval(value)))value = 0;
            disp_map_char_y_label.textContent = `Character Y: ${value}`;
            selection_window['Character Y'] = value;
            data['Character Y'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_map_char_y_input);
    const disp_map_char_sx_label = document.createElement("label");
    disp_map_char_sx_label.id = "Window_Label";
    disp_map_char_sx_label.textContent = `Character Scale X: ${selec_window['Character Scale X']}`;
    selc_window_container.appendChild(disp_map_char_sx_label);
    const disp_map_char_sx_input = document.createElement("input");
    disp_map_char_sx_input.id = "Window_Input";
    disp_map_char_sx_input.setAttribute("value", selec_window['Character Scale X']);
    disp_map_char_sx_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_map_char_sx_input.value;
            if(isNaN(eval(value)))value = 1;
            disp_map_char_sx_label.textContent = `Character Scale X: ${value}`;
            selection_window['Character Scale X'] = value;
            data['Character Scale X'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_map_char_sx_input);
    const disp_map_char_sy_label = document.createElement("label");
    disp_map_char_sy_label.id = "Window_Label";
    disp_map_char_sy_label.textContent = `Character Scale Y: ${selec_window['Character Scale Y']}`;
    selc_window_container.appendChild(disp_map_char_sy_label);
    const disp_map_char_sy_input = document.createElement("input");
    disp_map_char_sy_input.id = "Window_Input";
    disp_map_char_sy_input.setAttribute("value", selec_window['Character Scale Y']);
    disp_map_char_sy_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_map_char_sy_input.value;
            if(isNaN(eval(value)))value = 1;
            disp_map_char_sy_label.textContent = `Character Scale Y: ${value}`;
            selection_window['Character Scale Y'] = value;
            data['Character Scale Y'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_map_char_sy_input);
    const disp_batlr_label = document.createElement('label');
    disp_batlr_label.id = "Window_Label";
    disp_batlr_label.textContent = `Display Battler: ${eval(selec_window['Display Battler'])}`;
    selc_window_container.appendChild(disp_batlr_label);
    const disp_batlr_input = document.createElement("input");
    disp_batlr_input.id = "Window_Input";
    disp_batlr_input.setAttribute("type", "checkbox");
    disp_batlr_input.setAttribute("checked", eval(selec_window['Display Battler']));
    disp_batlr_input.checked = eval(selec_window['Display Battler']);
    disp_batlr_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_batlr_input.checked;
            disp_map_char_label.textContent = `Display Battler: ${value}`;
            selection_window['Display Battler'] = value.toString();
            data['Display Battler'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    selc_window_container.appendChild(disp_batlr_input);
    const disp_batlr_mot_label = document.createElement("label");
    disp_batlr_mot_label.id = "Window_Label";
    disp_batlr_mot_label.textContent = `Battler Motion: ${selec_window['Battler Motion']}`;
    selc_window_container.appendChild(disp_batlr_mot_label);
    const disp_batlr_mot_input = document.createElement("input");
    disp_batlr_mot_input.id = "Window_Input";
    disp_batlr_mot_input.setAttribute("value", selec_window['Battler Motion']);
    disp_batlr_mot_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_batlr_mot_input.value;
            value = value.toString().toLowerCase();
            disp_map_char_dir_label.textContent = `Battler Motion: ${value}`;
            selection_window['Battler Motion'] = value;
            data['Battler Motion'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_batlr_mot_input)
    const disp_batlr_x_label = document.createElement("label");
    disp_batlr_x_label.id = "Window_Label";
    disp_batlr_x_label.textContent = `Battler X: ${selec_window['Battler X']}`;
    selc_window_container.appendChild(disp_batlr_x_label);
    const disp_batlr_x_input = document.createElement("input");
    disp_batlr_x_input.id = "Window_Input";
    disp_batlr_x_input.setAttribute("value", selec_window['Battler X']);
    disp_batlr_x_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_batlr_x_input.value;
            if(isNaN(eval(value)))value = 0;
            disp_batlr_x_label.textContent = `Battler X: ${value}`;
            selection_window['Battler X'] = value;
            data['Battler X'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_batlr_x_input);
    const disp_batlr_y_label = document.createElement("label");
    disp_batlr_y_label.id = "Window_Label";
    disp_batlr_y_label.textContent = `Battler Y: ${selec_window['Battler Y']}`;
    selc_window_container.appendChild(disp_batlr_y_label);
    const disp_batlr_y_input = document.createElement("input");
    disp_batlr_y_input.id = "Window_Input";
    disp_batlr_y_input.setAttribute("value", selec_window['Battler Y']);
    disp_batlr_y_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_batlr_y_input.value;
            if(isNaN(eval(value)))value = 0;
            disp_batlr_y_label.textContent = `Battler Y: ${value}`;
            selection_window['Battler Y'] = value;
            data['Battler Y'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_batlr_y_input);
    const disp_batlr_sx_label = document.createElement("label");
    disp_batlr_sx_label.id = "Window_Label";
    disp_batlr_sx_label.textContent = `Battler Scale X: ${selec_window['Battler Scale X']}`;
    selc_window_container.appendChild(disp_batlr_sx_label);
    const disp_batlr_sx_input = document.createElement("input");
    disp_batlr_sx_input.id = "Window_Input";
    disp_batlr_sx_input.setAttribute("value", selec_window['Battler Scale X']);
    disp_batlr_sx_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_batlr_sx_input.value;
            if(isNaN(eval(value)))value = 1;
            disp_batlr_sx_label.textContent = `Battler Scale X: ${value}`;
            selection_window['Battler Scale X'] = value;
            data['Battler Scale X'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_batlr_sx_input);
    const disp_batlr_sy_label = document.createElement("label");
    disp_batlr_sy_label.id = "Window_Label";
    disp_batlr_sy_label.textContent = `Battler Scale Y: ${selec_window['Battler Scale Y']}`;
    selc_window_container.appendChild(disp_batlr_sy_label);
    const disp_batlr_sy_input = document.createElement("input");
    disp_batlr_sy_input.id = "Window_Input";
    disp_batlr_sy_input.setAttribute("value", selec_window['Battler Scale Y']);
    disp_batlr_sy_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const selection_window = menu['Actor Selection Window'];
        if(selection_window){
            let value = disp_batlr_sy_input.value;
            if(isNaN(eval(value)))value = 1;
            disp_batlr_sy_label.textContent = `Battler Scale Y: ${value}`;
            selection_window['Battler Scale Y'] = value;
            data['Battler Scale Y'] = value;
            recompileMenus(menu_data, menu);
        }    
    })
    selc_window_container.appendChild(disp_batlr_sy_input);
    form.appendChild(selc_window_container);
}

const createActrDataWindowsForm = function(form, data){
    const data_windows = data['Actor Data Windows'];
    let index = 0;
    const collapse_button = document.createElement('button');
    collapse_button.id = 'open_list_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = "Actor Data Windows";
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.id === "Actor_Data_Windows_Container"){
            if(content.style.display === "block"){
                content.style.display = "none";
            }else{
                content.style.display = "block";
            }
        }
    })
    form.appendChild(collapse_button);
    const windows_container = document.createElement('div');
    windows_container.id = "Actor_Data_Windows_Container";
    windows_container.classList.add("content");
    form.appendChild(windows_container);
    data_windows.forEach((data_window)=>{
        const collapse_button = document.createElement('button');
        collapse_button.id = 'open_list_button';
        collapse_button.classList.add('collapsible');
        collapse_button.type = 'button';
        collapse_button.textContent = `${data_window['Name']}`;
        collapse_button.addEventListener('click', function(){
            this.classList.toggle("active");
            const content = this.nextElementSibling;
            if(content.id === "Data_Window_Container"){
                if(content.style.display === "block"){
                    content.style.display = "none";
                }else{
                    content.style.display = "block";
                }
            }
        })
        windows_container.appendChild(collapse_button);
        const selec_window = data['Actor Data Windows'][index];
        const selc_window_container = document.createElement('div');
        selc_window_container.id = "Data_Window_Container";
        selc_window_container.classList.add("content");
        windows_container.appendChild(selc_window_container);
        const window_name_label = document.createElement('label');
        window_name_label.id = "Window_Label";
        window_name_label.textContent = `Window Name: ${data_window['Name']}`;
        selc_window_container.appendChild(window_name_label);
        const window_name_input = document.createElement("input");
        window_name_input.id = "Window_Input";
        window_name_input.setAttribute("value", `${data_window['Name']}`);
        window_name_input._saved_index = JSON.parse(JSON.stringify(index));
        window_name_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = window_name_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                const value = window_name_input.value;
                collapse_button.textContent = `${value}`
                window_name_label.textContent = `Window Name: ${value}`;
                selection_window['Name'] = value.toString();
                data_window['Name'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(window_name_input);
        addActrDataWindowDimensionForm(selc_window_container, data, index);
        addActrDataWindowStyleForm(selc_window_container, data, index);
        addActrWindowDataRequirementsForm(selc_window_container, data, index);
        addActrDataWindowGaugesForm(selc_window_container, data, index);
        const drw_actr_name_label = document.createElement('label');
        drw_actr_name_label.id = "Window_Label";
        drw_actr_name_label.textContent = `Draw Actor Name: ${eval(selec_window['Draw Actor Name']) ? "true" : "false"}`;
        selc_window_container.appendChild(drw_actr_name_label);
        const drw_actr_name_input = document.createElement("input");
        drw_actr_name_input.id = "Window_Input";
        drw_actr_name_input.setAttribute("type", "checkbox");
        drw_actr_name_input.setAttribute("checked", eval(selec_window['Draw Actor Name']));
        drw_actr_name_input.checked = eval(selec_window['Draw Actor Name']);
        drw_actr_name_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_actr_name_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_actr_name_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_actr_name_input.checked;
                drw_actr_name_label.textContent = `Draw Actor Name: ${value}`;
                selection_window['Draw Actor Name'] = value.toString();
                data_window['Draw Actor Name'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_actr_name_input);
        const drw_name_text_label = document.createElement('label');
        drw_name_text_label.id = "Window_Label";
        drw_name_text_label.textContent = `Name Text: %1 = Actor Name`;
        selc_window_container.appendChild(drw_name_text_label);
        const drw_name_text_input = document.createElement("input");
        drw_name_text_input.id = "Window_Input";
        drw_name_text_input.setAttribute("type", "text");
        drw_name_text_input.setAttribute("value", selec_window['Name Text']);
        drw_name_text_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_name_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_name_text_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_name_text_input.value;
                drw_name_text_label.textContent = `Name Text: %1 = Actor Name`;
                selection_window['Name Text'] = value;
                data_window['Name Text'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_name_text_input);
        const drw_name_x_label = document.createElement('label');
        drw_name_x_label.id = "Window_Label";
        drw_name_x_label.textContent = `Name X: ${selec_window['Name X']}`;
        selc_window_container.appendChild(drw_name_x_label);
        const drw_name_x_input = document.createElement("input");
        drw_name_x_input.id = "Window_Input";
        drw_name_x_input.setAttribute("type", "text");
        drw_name_x_input.setAttribute("value", selec_window['Name X']);
        drw_name_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_name_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_name_x_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_name_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_name_x_label.textContent = `Name X: ${value}`;
                selection_window['Name X'] = value;
                data_window['Name X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_name_x_input);
        const drw_name_y_label = document.createElement('label');
        drw_name_y_label.id = "Window_Label";
        drw_name_y_label.textContent = `Name Y: ${selec_window['Name Y']}`;
        selc_window_container.appendChild(drw_name_y_label);
        const drw_name_y_input = document.createElement("input");
        drw_name_y_input.id = "Window_Input";
        drw_name_y_input.setAttribute("type", "text");
        drw_name_y_input.setAttribute("value", selec_window['Name Y']);
        drw_name_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_name_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_name_y_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_name_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_name_y_label.textContent = `Name Y: ${value}`;
                selection_window['Name Y'] = value;
                data_window['Name Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_name_y_input);
        const drw_actr_profile_label = document.createElement('label');
        drw_actr_profile_label.id = "Window_Label";
        drw_actr_profile_label.textContent = `Draw Actor Profile: ${eval(selec_window['Draw Actor Profile']) ? "true" : "false"}`;
        selc_window_container.appendChild(drw_actr_profile_label);
        const drw_actr_profile_input = document.createElement("input");
        drw_actr_profile_input.id = "Window_Input";
        drw_actr_profile_input.setAttribute("type", "checkbox");
        drw_actr_profile_input.setAttribute("checked", eval(selec_window['Draw Actor Profile']));
        drw_actr_profile_input.checked = eval(selec_window['Draw Actor Profile']);
        drw_actr_profile_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_actr_profile_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_actr_profile_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_actr_profile_input.checked;
                drw_actr_profile_label.textContent = `Draw Actor Profile: ${value}`;
                selection_window['Draw Actor Name'] = value.toString();
                data_window['Draw Actor Name'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_actr_profile_input);
        const drw_profile_x_label = document.createElement('label');
        drw_profile_x_label.id = "Window_Label";
        drw_profile_x_label.textContent = `Profile X: ${selec_window['Profile X']}`;
        selc_window_container.appendChild(drw_profile_x_label);
        const drw_profile_x_input = document.createElement("input");
        drw_profile_x_input.id = "Window_Input";
        drw_profile_x_input.setAttribute("type", "text");
        drw_profile_x_input.setAttribute("value", selec_window['Profile X']);
        drw_profile_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_profile_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_profile_x_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_profile_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_profile_x_label.textContent = `Profile X: ${value}`;
                selection_window['Profile X'] = value.toString();
                data_window['Profile X'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_profile_x_input);
        const drw_profile_y_label = document.createElement('label');
        drw_profile_y_label.id = "Window_Label";
        drw_profile_y_label.textContent = `Profile Y: ${selec_window['Profile Y']}`;
        selc_window_container.appendChild(drw_profile_y_label);
        const drw_profile_y_input = document.createElement("input");
        drw_profile_y_input.id = "Window_Input";
        drw_profile_y_input.setAttribute("type", "text");
        drw_profile_y_input.setAttribute("value", selec_window['Profile Y']);
        drw_profile_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_profile_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_profile_y_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_profile_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_profile_y_label.textContent = `Profile Y: ${value}`;
                selection_window['Profile Y'] = value;
                data_window['Profile Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_profile_y_input);
        const drw_cls_lvl_label = document.createElement('label');
        drw_cls_lvl_label.id = "Window_Label";
        drw_cls_lvl_label.textContent = `Draw Class Level: ${eval(selec_window['Draw Class Level']) ? "true" : "false"}`;
        selc_window_container.appendChild(drw_cls_lvl_label);
        const drw_cls_lvl_input = document.createElement("input");
        drw_cls_lvl_input.id = "Window_Input";
        drw_cls_lvl_input.setAttribute("type", "checkbox");
        drw_cls_lvl_input.setAttribute("checked", eval(selec_window['Draw Class Level']));
        drw_cls_lvl_input.checked = eval(selec_window['Draw Class Level']);
        drw_cls_lvl_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_cls_lvl_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_cls_lvl_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_cls_lvl_input.checked;
                drw_cls_lvl_label.textContent = `Draw Class Level: ${value}`;
                selection_window['Draw Class Level'] = value.toString();
                data_window['Draw Class Level'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_cls_lvl_input);
        const drw_cls_lvl_text_label = document.createElement('label');
        drw_cls_lvl_text_label.id = "Window_Label";
        drw_cls_lvl_text_label.textContent = `Class Level Text: %1 = Class Name`;
        selc_window_container.appendChild(drw_cls_lvl_text_label);
        const drw_cls_lvl_text_input = document.createElement("input");
        drw_cls_lvl_text_input.id = "Window_Input";
        drw_cls_lvl_text_input.setAttribute("type", "text");
        drw_cls_lvl_text_input.setAttribute("value", selec_window['Class Level Text']);
        drw_cls_lvl_text_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_cls_lvl_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_cls_lvl_text_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_cls_lvl_text_input.value;
                drw_cls_lvl_text_label.textContent = `Class Level Text: %1 = Class Name`;
                selection_window['Class Level Text'] = value;
                data_window['Class Level Text'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_cls_lvl_text_input);
        const drw_cls_lvl_x_label = document.createElement('label');
        drw_cls_lvl_x_label.id = "Window_Label";
        drw_cls_lvl_x_label.textContent = `Class Level X: ${selec_window['Class Level X']}`;
        selc_window_container.appendChild(drw_cls_lvl_x_label);
        const drw_cls_lvl_x_input = document.createElement("input");
        drw_cls_lvl_x_input.id = "Window_Input";
        drw_cls_lvl_x_input.setAttribute("type", "text");
        drw_cls_lvl_x_input.setAttribute("value", selec_window['Class Level X']);
        drw_cls_lvl_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_cls_lvl_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_cls_lvl_x_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_cls_lvl_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_cls_lvl_x_label.textContent = `Class Level X: ${value}`;
                selection_window['Class Level X'] = value;
                data_window['Class Level X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_cls_lvl_x_input);
        const drw_cls_lvl_y_label = document.createElement('label');
        drw_cls_lvl_y_label.id = "Window_Label";
        drw_cls_lvl_y_label.textContent = `Class Level Y: ${selec_window['Class Level Y']}`;
        selc_window_container.appendChild(drw_cls_lvl_y_label);
        const drw_cls_lvl_y_input = document.createElement("input");
        drw_cls_lvl_y_input.id = "Window_Input";
        drw_cls_lvl_y_input.setAttribute("type", "text");
        drw_cls_lvl_y_input.setAttribute("value", selec_window['Class Level Y']);
        drw_cls_lvl_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_cls_lvl_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_cls_lvl_y_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_cls_lvl_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_cls_lvl_y_label.textContent = `Class Level Y: ${value}`;
                selection_window['Class Level Y'] = value;
                data_window['Class Level Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_cls_lvl_y_input);
        const drw_hp_res_label = document.createElement('label');
        drw_hp_res_label.id = "Window_Label";
        drw_hp_res_label.textContent = `Draw HP Resource: ${eval(selec_window['Draw HP Resource']) ? "true" : "false"}`;
        selc_window_container.appendChild(drw_hp_res_label);
        const drw_hp_res_input = document.createElement("input");
        drw_hp_res_input.id = "Window_Input";
        drw_hp_res_input.setAttribute("type", "checkbox");
        drw_hp_res_input.setAttribute("checked", eval(selec_window['Draw HP Resource']));
        drw_hp_res_input.checked = eval(selec_window['Draw HP Resource']);
        drw_hp_res_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_hp_res_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_hp_res_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_hp_res_input.checked;
                drw_hp_res_label.textContent = `Draw HP Resource: ${value}`;
                selection_window['Draw HP Resource'] = value.toString();
                data_window['Draw HP Resource'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_hp_res_input);
        const drw_hp_res_text_label = document.createElement('label');
        drw_hp_res_text_label.id = "Window_Label";
        drw_hp_res_text_label.textContent = `HP Text: %1 = Current, %2 = Max`;
        selc_window_container.appendChild(drw_hp_res_text_label);
        const drw_hp_res_text_input = document.createElement("input");
        drw_hp_res_text_input.id = "Window_Input";
        drw_hp_res_text_input.setAttribute("type", "text");
        drw_hp_res_text_input.setAttribute("value", selec_window['HP Text']);
        drw_hp_res_text_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_hp_res_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_hp_res_text_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_hp_res_text_input.value;
                drw_hp_res_text_label.textContent = `HP Text: %1 = Current, %2 = Max`;
                selection_window['HP Text'] = value;
                data_window['HP Text'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_hp_res_text_input);
        const drw_hp_res_x_label = document.createElement('label');
        drw_hp_res_x_label.id = "Window_Label";
        drw_hp_res_x_label.textContent = `HP X: ${selec_window['HP X']}`;
        selc_window_container.appendChild(drw_hp_res_x_label);
        const drw_hp_res_x_input = document.createElement("input");
        drw_hp_res_x_input.id = "Window_Input";
        drw_hp_res_x_input.setAttribute("type", "text");
        drw_hp_res_x_input.setAttribute("value", selec_window['HP X']);
        drw_hp_res_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_hp_res_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_hp_res_x_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_hp_res_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_hp_res_x_label.textContent = `HP X: ${value}`;
                selection_window['HP X'] = value;
                data_window['HP X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_hp_res_x_input);
        const drw_hp_res_y_label = document.createElement('label');
        drw_hp_res_y_label.id = "Window_Label";
        drw_hp_res_y_label.textContent = `HP Y: ${selec_window['HP Y']}`;
        selc_window_container.appendChild(drw_hp_res_y_label);
        const drw_hp_res_y_input = document.createElement("input");
        drw_hp_res_y_input.id = "Window_Input";
        drw_hp_res_y_input.setAttribute("type", "text");
        drw_hp_res_y_input.setAttribute("value", selec_window['HP Y']);
        drw_hp_res_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_hp_res_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_hp_res_y_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_hp_res_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_hp_res_y_label.textContent = `HP Y: ${value}`;
                selection_window['HP Y'] = value;
                data_window['HP Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_hp_res_y_input);
        const drw_mp_res_label = document.createElement('label');
        drw_mp_res_label.id = "Window_Label";
        drw_mp_res_label.textContent = `Draw MP Resource: ${eval(selec_window['Draw MP Resource']) ? "true" : "false"}`;
        selc_window_container.appendChild(drw_mp_res_label);
        const drw_mp_res_input = document.createElement("input");
        drw_mp_res_input.id = "Window_Input";
        drw_mp_res_input.setAttribute("type", "checkbox");
        drw_mp_res_input.setAttribute("checked", eval(selec_window['Draw MP Resource']));
        drw_mp_res_input.checked = eval(selec_window['Draw MP Resource']);
        drw_mp_res_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_mp_res_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_mp_res_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_mp_res_input.checked;
                drw_mp_res_label.textContent = `Draw MP Resource: ${value}`;
                selection_window['Draw MP Resource'] = value.toString();
                data_window['Draw MP Resource'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_mp_res_input);
        const drw_mp_res_text_label = document.createElement('label');
        drw_mp_res_text_label.id = "Window_Label";
        drw_mp_res_text_label.textContent = `MP Text: %1 = Current, %2 = Max`;
        selc_window_container.appendChild(drw_mp_res_text_label);
        const drw_mp_res_text_input = document.createElement("input");
        drw_mp_res_text_input.id = "Window_Input";
        drw_mp_res_text_input.setAttribute("type", "text");
        drw_mp_res_text_input.setAttribute("value", selec_window['MP Text']);
        drw_mp_res_text_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_mp_res_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_mp_res_text_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_mp_res_text_input.value;
                drw_mp_res_text_label.textContent = `MP Text: %1 = Current, %2 = Max`;
                selection_window['MP Text'] = value;
                data_window['MP Text'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_mp_res_text_input);
        const drw_mp_res_x_label = document.createElement('label');
        drw_mp_res_x_label.id = "Window_Label";
        drw_mp_res_x_label.textContent = `MP X: ${selec_window['MP X']}`;
        selc_window_container.appendChild(drw_mp_res_x_label);
        const drw_mp_res_x_input = document.createElement("input");
        drw_mp_res_x_input.id = "Window_Input";
        drw_mp_res_x_input.setAttribute("type", "text");
        drw_mp_res_x_input.setAttribute("value", selec_window['MP X']);
        drw_mp_res_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_mp_res_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_mp_res_x_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_mp_res_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_mp_res_x_label.textContent = `MP X: ${value}`;
                selection_window['MP X'] = value;
                data_window['MP X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_mp_res_x_input);
        const drw_mp_res_y_label = document.createElement('label');
        drw_mp_res_y_label.id = "Window_Label";
        drw_mp_res_y_label.textContent = `MP Y: ${selec_window['MP Y']}`;
        selc_window_container.appendChild(drw_mp_res_y_label);
        const drw_mp_res_y_input = document.createElement("input");
        drw_mp_res_y_input.id = "Window_Input";
        drw_mp_res_y_input.setAttribute("type", "text");
        drw_mp_res_y_input.setAttribute("value", selec_window['MP Y']);
        drw_mp_res_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_mp_res_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_mp_res_y_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_mp_res_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_mp_res_y_label.textContent = `MP Y: ${value}`;
                selection_window['MP Y'] = value;
                data_window['MP Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_mp_res_y_input);
        const drw_tp_res_label = document.createElement('label');
        drw_tp_res_label.id = "Window_Label";
        drw_tp_res_label.textContent = `Draw TP Resource: ${eval(selec_window['Draw TP Resource']) ? "true" : "false"}`;
        selc_window_container.appendChild(drw_tp_res_label);
        const drw_tp_res_input = document.createElement("input");
        drw_tp_res_input.id = "Window_Input";
        drw_tp_res_input.setAttribute("type", "checkbox");
        drw_tp_res_input.setAttribute("checked", eval(selec_window['Draw TP Resource']));
        drw_tp_res_input.checked = eval(selec_window['Draw TP Resource']);
        drw_tp_res_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_tp_res_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_tp_res_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_tp_res_input.checked;
                drw_tp_res_label.textContent = `Draw TP Resource: ${value}`;
                selection_window['Draw TP Resource'] = value.toString();
                data_window['Draw TP Resource'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_tp_res_input);
        const drw_tp_res_text_label = document.createElement('label');
        drw_tp_res_text_label.id = "Window_Label";
        drw_tp_res_text_label.textContent = `TP Text: %1 = Current, %2 = Max`;
        selc_window_container.appendChild(drw_tp_res_text_label);
        const drw_tp_res_text_input = document.createElement("input");
        drw_tp_res_text_input.id = "Window_Input";
        drw_tp_res_text_input.setAttribute("type", "text");
        drw_tp_res_text_input.setAttribute("value", selec_window['TP Text']);
        drw_tp_res_text_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_tp_res_text_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_tp_res_text_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_tp_res_text_input.value;
                drw_tp_res_text_label.textContent = `TP Text: %1 = Current, %2 = Max`;
                selection_window['TP Text'] = value;
                data_window['TP Text'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_tp_res_text_input);
        const drw_tp_res_x_label = document.createElement('label');
        drw_tp_res_x_label.id = "Window_Label";
        drw_tp_res_x_label.textContent = `TP X: ${selec_window['TP X']}`;
        selc_window_container.appendChild(drw_tp_res_x_label);
        const drw_tp_res_x_input = document.createElement("input");
        drw_tp_res_x_input.id = "Window_Input";
        drw_tp_res_x_input.setAttribute("type", "text");
        drw_tp_res_x_input.setAttribute("value", selec_window['TP X']);
        drw_tp_res_x_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_tp_res_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_tp_res_x_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_tp_res_x_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_tp_res_x_label.textContent = `TP X: ${value}`;
                selection_window['TP X'] = value;
                data_window['TP X'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_tp_res_x_input);
        const drw_tp_res_y_label = document.createElement('label');
        drw_tp_res_y_label.id = "Window_Label";
        drw_tp_res_y_label.textContent = `TP Y: ${selec_window['TP Y']}`;
        selc_window_container.appendChild(drw_tp_res_y_label);
        const drw_tp_res_y_input = document.createElement("input");
        drw_tp_res_y_input.id = "Window_Input";
        drw_tp_res_y_input.setAttribute("type", "text");
        drw_tp_res_y_input.setAttribute("value", selec_window['TP Y']);
        drw_tp_res_y_input._saved_index = JSON.parse(JSON.stringify(index));
        drw_tp_res_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = drw_tp_res_y_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = drw_tp_res_y_input.value;
                if(isNaN(eval(value)))value = 0;
                drw_tp_res_y_label.textContent = `TP Y: ${value}`;
                selection_window['TP Y'] = value;
                data_window['TP Y'] = value;
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(drw_tp_res_y_input);
        addActrDataBaseParamForm(selc_window_container, data, index);
        addActrDataExParamForm(selc_window_container, data, index);
        addActrDataSpParamForm(selc_window_container, data, index);
        const disp_map_char_label = document.createElement('label');
        disp_map_char_label.id = "Window_Label";
        disp_map_char_label.textContent = `Display Map Character: ${eval(selec_window['Display Map Character'])}`;
        selc_window_container.appendChild(disp_map_char_label);
        const disp_map_char_input = document.createElement("input");
        disp_map_char_input.id = "Window_Input";
        disp_map_char_input.setAttribute("type", "checkbox");
        disp_map_char_input.setAttribute("checked", eval(selec_window['Display Map Character']));
        disp_map_char_input.checked = eval(selec_window['Display Map Character']);
        disp_map_char_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_map_char_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_map_char_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_map_char_input.checked;
                disp_map_char_label.textContent = `Display Map Character: ${value}`;
                selection_window['Display Map Character'] = value.toString();
                data_window['Display Map Character'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(disp_map_char_input);
        const disp_map_char_dir_label = document.createElement("label");
        disp_map_char_dir_label.id = "Window_Label";
        disp_map_char_dir_label.textContent = `Character Direction: ${dirToString(selec_window['Character Direction'])}`;
        selc_window_container.appendChild(disp_map_char_dir_label);
        const disp_map_char_dir_input = document.createElement("input");
        disp_map_char_dir_input.id = "Window_Input";
        disp_map_char_dir_input.setAttribute("value", selec_window['Character Direction']);
        disp_map_char_dir_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_map_char_dir_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_map_char_dir_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_map_char_dir_input.value;
                if(isNaN(eval(value)))value = 0;
                disp_map_char_dir_label.textContent = `Character Direction: ${dirToString(value)}`;
                selection_window['Character Direction'] = value;
                data_window['Character Direction'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_map_char_dir_input)
        const disp_map_char_x_label = document.createElement("label");
        disp_map_char_x_label.id = "Window_Label";
        disp_map_char_x_label.textContent = `Character X: ${selec_window['Character X']}`;
        selc_window_container.appendChild(disp_map_char_x_label);
        const disp_map_char_x_input = document.createElement("input");
        disp_map_char_x_input.id = "Window_Input";
        disp_map_char_x_input.setAttribute("value", selec_window['Character X']);
        disp_map_char_x_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_map_char_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_map_char_x_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_map_char_x_input.value;
                if(isNaN(eval(value)))value = 0;
                disp_map_char_x_label.textContent = `Character X: ${value}`;
                selection_window['Character X'] = value;
                data_window['Character X'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_map_char_x_input);
        const disp_map_char_y_label = document.createElement("label");
        disp_map_char_y_label.id = "Window_Label";
        disp_map_char_y_label.textContent = `Character Y: ${selec_window['Character Y']}`;
        selc_window_container.appendChild(disp_map_char_y_label);
        const disp_map_char_y_input = document.createElement("input");
        disp_map_char_y_input.id = "Window_Input";
        disp_map_char_y_input.setAttribute("value", selec_window['Character Y']);
        disp_map_char_y_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_map_char_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_map_char_y_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_map_char_y_input.value;
                if(isNaN(eval(value)))value = 0;
                disp_map_char_y_label.textContent = `Character Y: ${value}`;
                selection_window['Character Y'] = value;
                data_window['Character Y'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_map_char_y_input);
        const disp_map_char_sx_label = document.createElement("label");
        disp_map_char_sx_label.id = "Window_Label";
        disp_map_char_sx_label.textContent = `Character Scale X: ${selec_window['Character Scale X']}`;
        selc_window_container.appendChild(disp_map_char_sx_label);
        const disp_map_char_sx_input = document.createElement("input");
        disp_map_char_sx_input.id = "Window_Input";
        disp_map_char_sx_input.setAttribute("value", selec_window['Character Scale X']);
        disp_map_char_sx_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_map_char_sx_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_map_char_sx_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_map_char_sx_input.value;
                if(isNaN(eval(value)))value = 1;
                disp_map_char_sx_label.textContent = `Character Scale X: ${value}`;
                selection_window['Character Scale X'] = value;
                data_window['Character Scale X'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_map_char_sx_input);
        const disp_map_char_sy_label = document.createElement("label");
        disp_map_char_sy_label.id = "Window_Label";
        disp_map_char_sy_label.textContent = `Character Scale Y: ${selec_window['Character Scale Y']}`;
        selc_window_container.appendChild(disp_map_char_sy_label);
        const disp_map_char_sy_input = document.createElement("input");
        disp_map_char_sy_input.id = "Window_Input";
        disp_map_char_sy_input.setAttribute("value", selec_window['Character Scale Y']);
        disp_map_char_sy_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_map_char_sy_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_map_char_sy_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_map_char_sy_input.value;
                if(isNaN(eval(value)))value = 1;
                disp_map_char_sy_label.textContent = `Character Scale Y: ${value}`;
                selection_window['Character Scale Y'] = value;
                data_window['Character Scale Y'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_map_char_sy_input);
        const disp_batlr_label = document.createElement('label');
        disp_batlr_label.id = "Window_Label";
        disp_batlr_label.textContent = `Display Battler: ${eval(selec_window['Display Battler'])}`;
        selc_window_container.appendChild(disp_batlr_label);
        const disp_batlr_input = document.createElement("input");
        disp_batlr_input.id = "Window_Input";
        disp_batlr_input.setAttribute("type", "checkbox");
        disp_batlr_input.setAttribute("checked", eval(selec_window['Display Battler']));
        disp_batlr_input.checked = eval(selec_window['Display Battler']);
        disp_batlr_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_batlr_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_batlr_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_batlr_input.checked;
                disp_map_char_label.textContent = `Display Battler: ${value}`;
                selection_window['Display Battler'] = value.toString();
                data_window['Display Battler'] = value.toString();
                recompileMenus(menu_data, menu);
            }
        })
        selc_window_container.appendChild(disp_batlr_input);
        const disp_batlr_mot_label = document.createElement("label");
        disp_batlr_mot_label.id = "Window_Label";
        disp_batlr_mot_label.textContent = `Battler Motion: ${selec_window['Battler Motion']}`;
        selc_window_container.appendChild(disp_batlr_mot_label);
        const disp_batlr_mot_input = document.createElement("input");
        disp_batlr_mot_input.id = "Window_Input";
        disp_batlr_mot_input.setAttribute("value", selec_window['Battler Motion']);
        disp_batlr_mot_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_batlr_mot_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_batlr_mot_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_batlr_mot_input.value;
                value = value.toString().toLowerCase();
                disp_map_char_dir_label.textContent = `Battler Motion: ${value}`;
                selection_window['Battler Motion'] = value;
                data_window['Battler Motion'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_batlr_mot_input)
        const disp_batlr_x_label = document.createElement("label");
        disp_batlr_x_label.id = "Window_Label";
        disp_batlr_x_label.textContent = `Battler X: ${selec_window['Battler X']}`;
        selc_window_container.appendChild(disp_batlr_x_label);
        const disp_batlr_x_input = document.createElement("input");
        disp_batlr_x_input.id = "Window_Input";
        disp_batlr_x_input.setAttribute("value", selec_window['Battler X']);
        disp_batlr_x_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_batlr_x_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_batlr_x_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_batlr_x_input.value;
                if(isNaN(eval(value)))value = 0;
                disp_batlr_x_label.textContent = `Battler X: ${value}`;
                selection_window['Battler X'] = value;
                data_window['Battler X'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_batlr_x_input);
        const disp_batlr_y_label = document.createElement("label");
        disp_batlr_y_label.id = "Window_Label";
        disp_batlr_y_label.textContent = `Battler Y: ${selec_window['Battler Y']}`;
        selc_window_container.appendChild(disp_batlr_y_label);
        const disp_batlr_y_input = document.createElement("input");
        disp_batlr_y_input.id = "Window_Input";
        disp_batlr_y_input.setAttribute("value", selec_window['Battler Y']);
        disp_batlr_y_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_batlr_y_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_batlr_y_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_batlr_y_input.value;
                if(isNaN(eval(value)))value = 0;
                disp_batlr_y_label.textContent = `Battler Y: ${value}`;
                selection_window['Battler Y'] = value;
                data_window['Battler Y'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_batlr_y_input);
        const disp_batlr_sx_label = document.createElement("label");
        disp_batlr_sx_label.id = "Window_Label";
        disp_batlr_sx_label.textContent = `Battler Scale X: ${selec_window['Battler Scale X']}`;
        selc_window_container.appendChild(disp_batlr_sx_label);
        const disp_batlr_sx_input = document.createElement("input");
        disp_batlr_sx_input.id = "Window_Input";
        disp_batlr_sx_input.setAttribute("value", selec_window['Battler Scale X']);
        disp_batlr_sx_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_batlr_sx_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_batlr_sx_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_batlr_sx_input.value;
                if(isNaN(eval(value)))value = 1;
                disp_batlr_sx_label.textContent = `Battler Scale X: ${value}`;
                selection_window['Battler Scale X'] = value;
                data_window['Battler Scale X'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_batlr_sx_input);
        const disp_batlr_sy_label = document.createElement("label");
        disp_batlr_sy_label.id = "Window_Label";
        disp_batlr_sy_label.textContent = `Battler Scale Y: ${selec_window['Battler Scale Y']}`;
        selc_window_container.appendChild(disp_batlr_sy_label);
        const disp_batlr_sy_input = document.createElement("input");
        disp_batlr_sy_input.id = "Window_Input";
        disp_batlr_sy_input.setAttribute("value", selec_window['Battler Scale Y']);
        disp_batlr_sy_input._saved_index = JSON.parse(JSON.stringify(index));
        disp_batlr_sy_input.addEventListener("input", ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = disp_batlr_sy_input._saved_index;
            const selection_window = menu['Actor Data Windows'][index];
            if(selection_window){
                let value = disp_batlr_sy_input.value;
                if(isNaN(eval(value)))value = 1;
                disp_batlr_sy_label.textContent = `Battler Scale Y: ${value}`;
                selection_window['Battler Scale Y'] = value;
                data_window['Battler Scale Y'] = value;
                recompileMenus(menu_data, menu);
            }    
        })
        selc_window_container.appendChild(disp_batlr_sy_input);
        const delete_button = document.createElement("button");
        delete_button.id = "del_btn";
        delete_button.type = "button";
        delete_button.textContent = "DEL";
        delete_button._saved_index = JSON.parse(JSON.stringify(index));
        delete_button.addEventListener(`click`, ()=>{
            const menu_data = window._menu_data;
            const menu = menu_data.find((menu)=>{
                return menu['Identifier Name'] == data['Identifier Name'];
            })
            const index = delete_button._saved_index;
            if(index >= 0){
                menu['Actor Data Windows'].splice(index, 1);
                recompileMenus(menu_data, menu, true);
            }
        })
        selc_window_container.appendChild(delete_button);
        index++;
    })
    const add_window_button = document.createElement('button');
    add_window_button.id = "Add_Button";
    add_window_button.type = 'button';
    add_window_button.textContent = "ADD ACTOR DATA WINDOW";
    add_window_button.addEventListener('click', ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        const windows = menu['Actor Data Windows'];
        let index = windows.length;
        const new_window = {
            "Battler Motion": "wait",
            "Battler Scale X": "1",
            "Battler Scale Y": "1",
            "Battler X": "0",
            "Battler Y": "0",
            "Character Direction": "2",
            "Character Scale X": "1",
            "Character Scale Y": "1",
            "Character X": "0",
            "Character Y": "0",
            "Class Level Text": "Class: %1 <%2>",
            "Class Level X": "0",
            "Class Level Y": "0",
            "Dimension Configuration": {
                "X": "0", 
                "Y": "0", 
                "Width": "1", 
                "Height": "1"
            },
            "Display Battler": "false",
            "Display Map Character": "false",
            "Display Requirements": "",
            "Draw Actor Name": false,
            "Draw Actor Profile": false,
            "Draw Base Params": [],
            "Draw Class Level": "false",
            "Draw Ex Params": [],
            "Draw HP Resource": "false",
            "Draw MP Resource": "false",
            "Draw Sp Params": [],
            "Draw TP Resource": "false",
            "Gauges": [],
            "HP Text": "\I[84]%1 / %2",
            "HP X": "0",
            "HP Y": "0",
            "MP Text": "\I[79]%1 / %2",
            "MP X": "0",
            "MP Y": "0",
            "Name": `New Window: ${index}`,
            "Name Text": "%1",
            "Name X": "0",
            "Name Y": "0",
            "Profile X": "0",
            "Profile Y": "0",
            "TP Text": "\I[79]%1 / %2",
            "TP X": "0",
            "TP Y": "0",
            "Window Font and Style Configuration": {
                "Font Settings": "", 
                "Font Size": "16", 
                "Font Face": "sans-serif", 
                "Base Font Color": "#ffffff", 
                "Font Outline Color": "rgba(0, 0, 0, 0.5)",
                "Font Outline Thickness": "3",
                "Window Skin": "Window",
                "Window Opacity": "255",
                "Show Window Dimmer": "false",
            }
        }
        windows.push(new_window);
        recompileMenus(menu_data, menu, true);
    })
    windows_container.appendChild(add_window_button);
}

const createMenuForm = function(container, data){
    const modification_form = document.createElement('form');
    modification_form.id = "Input_Form";
    container.appendChild(modification_form);
    createMenuIdNameForm(modification_form, data);
    createOnloadScriptsForm(modification_form, data);
    createBackgroundsForm(modification_form, data);
    createBackGfxForm(modification_form, data);
    createUpdateCodeForm(modification_form, data);
    createSelcWindowForm(modification_form, data);
    const open_selc_window_effect_label = document.createElement('label');
    open_selc_window_effect_label.id = "Window_Label";
    open_selc_window_effect_label.textContent = `Open Effect: ${data['Open Effect']}`;
    modification_form.appendChild(open_selc_window_effect_label);
    const open_selc_window_effect_input = document.createElement("input");
    open_selc_window_effect_input.id = "Window_Input";
    open_selc_window_effect_input.setAttribute("type", "checkbox");
    open_selc_window_effect_input.setAttribute("checked", eval(data['Open Effect']));
    open_selc_window_effect_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            const value = open_selc_window_effect_input.checked;
            open_selc_window_effect_label.textContent = `Open Effect: ${value ? "true" : "false"}`;
            data['Open Effect'] = value.toString();
            menu['Open Effect'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    modification_form.appendChild(open_selc_window_effect_input);
    const disable_cancel_btn_label = document.createElement('label');
    disable_cancel_btn_label.id = "Window_Label";
    disable_cancel_btn_label.textContent = `Disable Cancel Exit: ${data['Disable Cancel Exit']}`;
    modification_form.appendChild(disable_cancel_btn_label);
    const disable_cancel_btn_input = document.createElement("input");
    disable_cancel_btn_input.id = "Window_Input";
    disable_cancel_btn_input.setAttribute("type", "checkbox");
    disable_cancel_btn_input.setAttribute("checked", eval(data['Disable Cancel Exit']));
    disable_cancel_btn_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            const value = disable_cancel_btn_input.checked;
            disable_cancel_btn_label.textContent = `Disable Cancel Exit: ${value ? "true" : "false"}`;
            data['Disable Cancel Exit'] = value.toString();
            menu['Disable Cancel Exit'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    modification_form.appendChild(disable_cancel_btn_input);
    createActrSelcWindowForm(modification_form, data);
    const always_show_actr_selc_label = document.createElement('label');
    always_show_actr_selc_label.id = "Window_Label";
    always_show_actr_selc_label.textContent = `Always Show Actor Select: ${data['Always Show Actor Select']}`;
    modification_form.appendChild(always_show_actr_selc_label);
    const always_show_actr_selc_input = document.createElement("input");
    always_show_actr_selc_input.id = "Window_Input";
    always_show_actr_selc_input.setAttribute("type", "checkbox");
    always_show_actr_selc_input.setAttribute("checked", eval(data['Always Show Actor Select']));
    always_show_actr_selc_input.addEventListener("input", ()=>{
        const menu_data = window._menu_data;
        const menu = menu_data.find((menu)=>{
            return menu['Identifier Name'] == data['Identifier Name'];
        })
        if(menu){
            const value = always_show_actr_selc_input.checked;
            always_show_actr_selc_label.textContent = `Always Show Actor Select: ${value ? "true" : "false"}`;
            data['Always Show Actor Select'] = value.toString();
            menu['Always Show Actor Select'] = value.toString();
            recompileMenus(menu_data, menu);
        }
    })
    modification_form.appendChild(always_show_actr_selc_input);
    createActrDataWindowsForm(modification_form, data);
    createSelcDataWindowsForm(modification_form, data);
    createBasicWindowsForm(modification_form, data);
    createForeGfxForm(modification_form, data);
    createForegroundsForm(modification_form, data);
}

const addOverrideData = function(data, index){
    console.log(data);
    const contents = document.getElementById('contents');
    const collapse_button = document.createElement('button');
    collapse_button.id = 'menu_config_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = data['Name'];
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.style.display === "block"){
            content.style.display = "none";
        }else{
            content.style.display = "block";
        }
    })
    contents.appendChild(collapse_button);
    const collapse_div = document.createElement('div');
    collapse_div.id = 'menu_data_container';
    collapse_div.classList.add('content');
    contents.appendChild(collapse_div);
    const override_name_label = document.createElement("label");
    override_name_label.id = "Window_Label";
    override_name_label.textContent = `Name: ${data['Name']}`;
    collapse_div.appendChild(override_name_label);
    const override_name_input = document.createElement("input");
    override_name_input.id = "Window_Input";
    override_name_input.setAttribute("value", data['Name']);
    override_name_input._saved_index = JSON.parse(JSON.stringify(index));
    override_name_input.addEventListener("input", ()=>{
        const overrides = this._override_data;
        const index = override_name_input._saved_index;
        const override = overrides[index];
        if(!override)return;
        const value = override_name_input.value;
        override['Name'] = value;
        data['Name'] = value;
        override_name_label.textContent = `Name: ${value}`;
        collapse_button.textContent = value;
        modifyOverrides(overrides);
    })
    collapse_div.appendChild(override_name_input);
    const override_scene_label = document.createElement("label");
    override_scene_label.id = "Window_Label";
    override_scene_label.textContent = `Scene Class: ${data['Scene Class']}`;
    collapse_div.appendChild(override_scene_label);
    const override_scene_input = document.createElement("input");
    override_scene_input.id = "Window_Input";
    override_scene_input.setAttribute("value", data['Scene Class']);
    override_scene_input._saved_index = JSON.parse(JSON.stringify(index));
    override_scene_input.addEventListener("input", ()=>{
        const overrides = this._override_data;
        const index = override_scene_input._saved_index;
        const override = overrides[index];
        if(!override)return;
        const value = override_scene_input.value;
        override['Scene Class'] = value;
        data['Scene Class'] = value;
        override_scene_label.textContent = `Scene Class: ${value}`;
        collapse_button.textContent = value;
        modifyOverrides(overrides);
    })
    collapse_div.appendChild(override_scene_input);
    const override_menu_label = document.createElement("label");
    override_menu_label.id = "Window_Label";
    override_menu_label.textContent = `Menu Identifier: ${data['Menu Identifier']}`;
    collapse_div.appendChild(override_menu_label);
    const override_menu_input = document.createElement("input");
    override_menu_input.id = "Window_Input";
    override_menu_input.setAttribute("value", data['Menu Identifier']);
    override_menu_input._saved_index = JSON.parse(JSON.stringify(index));
    override_menu_input.addEventListener("input", ()=>{
        const overrides = this._override_data;
        const index = override_menu_input._saved_index;
        const override = overrides[index];
        if(!override)return;
        const value = override_menu_input.value;
        override['Menu Identifier'] = value;
        data['Menu Identifier'] = value;
        override_menu_label.textContent = `Menu Identifier: ${value}`;
        collapse_button.textContent = value;
        modifyOverrides(overrides);
    })
    collapse_div.appendChild(override_menu_input);
    const delete_button = document.createElement("button");
    delete_button.id = "del_btn";
    delete_button.type = "button";
    delete_button.textContent = "DEL";
    delete_button._saved_index = JSON.parse(JSON.stringify(index));
    delete_button.addEventListener(`click`, ()=>{
        const overrides = window._override_data;
        const index = delete_button._saved_index;
        const override = overrides[index];
        if(!override)return;
        overrides.splice(index, 1);
        modifyOverrides(overrides, true);
    })
    collapse_div.appendChild(delete_button);
}

const addMenuData = function(data, index){
    const contents = document.getElementById('contents');
    const collapse_button = document.createElement('button');
    collapse_button.id = 'menu_config_button';
    collapse_button.classList.add('collapsible');
    collapse_button.type = 'button';
    collapse_button.textContent = data['Identifier Name'];
    collapse_button.addEventListener('click', function(){
        this.classList.toggle("active");
        const content = this.nextElementSibling;
        if(content.style.display === "block"){
            content.style.display = "none";
        }else{
            content.style.display = "block";
        }
    })
    contents.appendChild(collapse_button);
    const collapse_div = document.createElement('div');
    collapse_div.id = 'menu_data_container';
    collapse_div.classList.add('content');
    contents.appendChild(collapse_div);
    createMenuForm(collapse_div, data);
    const delete_button = document.createElement("button");
    delete_button.id = "del_btn";
    delete_button.type = "button";
    delete_button.textContent = "DEL";
    delete_button._saved_index = JSON.parse(JSON.stringify(index));
    delete_button.addEventListener(`click`, ()=>{
        const menus = window._menu_data;
        const index = delete_button._saved_index;
        const menu = menus[index];
        if(!menu)return;
        menus.splice(index, 1);
        recompileMenus(menus, null, true);
    })
    collapse_div.appendChild(delete_button);
}

const createMenuListContents = function(){
    clearContents();
    const info_div = document.createElement('div');
    info_div.id = "Editor_Help";
    contents.appendChild(info_div);
    const info_msg1 = document.createElement('p');
    info_msg1.id = "Editor_Msg";
    info_msg1.textContent = "You can customize already existing menu below or add a new menu.";
    contents.appendChild(info_msg1);
    const plugin_data = window._menu_builder_plugin;
    const parameters = plugin_data.parameters;
    const menu_config_json = parameters['Menu Configurations'];
    const menu_configs = JSON.parse(menu_config_json);
    const menu_data = menu_configs.map((config)=>{
        return MENU_SCENE_BUILD_PARSER_MNUBLD(config);
    })
    let index = 0;
    menu_data.forEach((menu)=>{
        addMenuData(menu, index);
        index++;
    })
    const add_menu_button = document.createElement('button');
    add_menu_button.id = "Add_Button";
    add_menu_button.type = 'button';
    add_menu_button.textContent = "ADD MENU";
    add_menu_button.addEventListener('click', ()=>{
        const menus = this._menu_data;
        const index = menus.length;
        const obj = {};
        obj['Identifier Name'] = `Menu: ${index}`;
        obj['Preload Backgrounds'] = [];
        obj['On Load Script Calls'] = [];
        obj['Backgrounds'] = [];
        obj['Back Graphics'] = [];
        obj['Selection Window'] = {
            "Dimension Configuration":{
                "X":"0",
                "Y":"0",
                "Width":"1",
                "Height":"1"
            },
            "Window Font and Style Configuration":{
                "Font Settings":"",
                "Font Size":"16",
                "Font Face":"sans-serif",
                "Base Font Color":"#ffffff",
                "Font Outline Color":"rgba(0,0,0,0.5)",
                "Font Outline Thickness":"3",
                "Window Skin":"Window",
                "Window Opacity":"255",
                "Show Window Dimmer":"false"
            },
            "Item Width":"0",
            "Item Height":"0",
            "Max Columns":"0",
            "Selection Options":[],
            "Gauges":[],
            "Draw Name":"false",
            "Name Text":"%1",
            "Name X":"0",
            "Name Y":"0",
            "Draw Alternative Name":"false",
            "Alternative Name Text":"%1",
            "Alternative Name X":"0",
            "Alternative Name Y":"0",
            "Draw Description":"false",
            "Description X":"0",
            "Description Y":"0",
            "Draw Picture":"false",
            "Picture Index":"0",
            "Picture X":"0",
            "Picture Y":"0",
            "Picture Width":"0",
            "Picture Height":"0"
        };
        obj['Open Effect'] = "false";
        obj['Disable Cancel Exit'] = "false";
        obj['Actor Selection Window'] = {
            "Dimension Configuration":{
                "X":"0",
                "Y":"0",
                "Width":"1",
                "Height":"1"
            },
            "Window Font and Style Configuration":{
                "Font Settings":"",
                "Font Size":"16",
                "Font Face":"sans-serif",
                "Base Font Color":"#ffffff",
                "Font Outline Color":"rgba(0,0,0,0.5)",
                "Font Outline Thickness":"3",
                "Window Skin":"Window",
                "Window Opacity":"255",
                "Show Window Dimmer":"false"
            },
            "Item Width":"0",
            "Item Height":"0",
            "Max Columns":"0",
            "Gauges":[],
            "Draw Actor Name":"false",
            "Name Text":"%1",
            "Name X":"0",
            "Name Y":"0",
            "Draw Actor Profile":"false",
            "Profile X":"0",
            "Profile Y":"0",
            "Draw Class Level":"false",
            "Class Level Text":"%1",
            "Class Level X":"0",
            "Class Level Y":"0",
            "Draw HP Resource":"false",
            "HP Text":"%1",
            "HP X":"0",
            "HP Y":"0",
            "Draw MP Resource":"false",
            "MP Text":"%1",
            "MP X":"0",
            "MP Y":"0",
            "Draw TP Resource":"false",
            "TP Text":"%1",
            "TP X":"0",
            "TP Y":"0",
            "Draw Base Params":[],
            "Draw Ex Params":[],
            "Draw Sp Params":[],
            "Display Map Character":"false",
            "Character Direction":"2",
            "Character X":"0",
            "Character Y":"0",
            "Character Scale X":"1",
            "Character Scale Y":"1",
            "Display Battler":"false",
            "Battler Motion":"wait",
            "Battler X":"0",
            "Battler Y":"0",
            "Battler Scale X":"1",
            "Battler Scale Y":"1",
        };
        obj['Always Show Actor Select'] = "false";
        obj['Actor Data Windows'] = [];
        obj['Selection Data Windows'] = [];
        obj['Basic Windows'] = [];
        obj['Fore Graphics'] = [];
        obj['Foregrounds'] = [];
        menus.push(obj);
        recompileMenus(menus, null, true);
    })
    contents.appendChild(add_menu_button);
    this._menu_data = menu_data;
}

const reloadOverrideContents = function(plugin_data){
    window._menu_builder_plugin = plugin_data
    const parameters = plugin_data.parameters;
    const override_config_json = parameters['Scene Overrides'];
    const override_configs = JSON.parse(override_config_json);
    const override_data = override_configs.map((config)=>{
        return SCENE_OVERRIDE_PARSER_MNUBLD(config);
    })
    this._override_data = override_data;
}

const reloadMenuListContents = function(plugin_data){
    window._menu_builder_plugin = plugin_data
    const parameters = plugin_data.parameters;
    const menu_config_json = parameters['Menu Configurations'];
    const menu_configs = JSON.parse(menu_config_json);
    const menu_data = menu_configs.map((config)=>{
        return MENU_SCENE_BUILD_PARSER_MNUBLD(config);
    })
    this._menu_data = menu_data;
}

const createOverrideListContents = function(){
    clearContents();
    const info_div = document.createElement('div');
    info_div.id = "Editor_Help";
    contents.appendChild(info_div);
    const info_msg1 = document.createElement('p');
    info_msg1.id = "Editor_Msg";
    info_msg1.textContent = "You can customize already existing override below or add a new override.";
    contents.appendChild(info_msg1);
    const plugin_data = window._menu_builder_plugin;
    const parameters = plugin_data.parameters;
    const override_config_json = parameters['Scene Overrides'];
    const override_configs = JSON.parse(override_config_json);
    const override_data = override_configs.map((config)=>{
        return JSON.parse(config);
    })
    let index = 0;
    override_data.forEach((override)=>{
        addOverrideData(override, index);
        index++;
    })
    const add_override_button = document.createElement('button');
    add_override_button.id = "Add_Button";
    add_override_button.type = 'button';
    add_override_button.textContent = "ADD OVERRIDE";
    add_override_button.addEventListener('click', ()=>{
        const overrides = this._override_data;
        const index = overrides.length;
        overrides.push({
            "Name":`Override: ${index}`,
            "Scene Class":"",
            "Menu Identifier":""
        })
        modifyOverrides(overrides, true);
    })
    contents.appendChild(add_override_button);
    this._override_data = override_data;
}

const initDocument = function(){
    createTabs();
    createContents();
}

const load_menus = function(){
    clearDocument();
    initDocument();
}

const load_editor = function(){
    const body = document.getElementById('body');
    const source_window = window.opener;
    if(!source_window){
        const error_div = document.createElement('div');
        error_div.id = "error_div";
        body.appendChild(error_div);
        const msg = document.createElement('p');
        msg.id = "error_msg";
        msg.textContent = `Source window connection severed!\nUnable to connect plugin data.`;
        error_div.appendChild(msg);
        console.error(`Source window connection severed!\nUnable to connect plugin data.`)
        return;
    }
    const plugin_data = source_window.$plugins;
    const menu_builder_plugin_data = plugin_data.find((data)=>{
        return data.name == "Synrec_MenuBuilder"
    })
    if(!menu_builder_plugin_data){
        throw new Error(`Failed to load Synrec Menu Plugin Data`);
    }
    window._menu_builder_plugin = menu_builder_plugin_data;
    load_menus();
    const info_div = document.createElement('div');
    info_div.id = "Editor_Help";
    body.appendChild(info_div);
    const info_msg1 = document.createElement('p');
    info_msg1.id = "Editor_Msg";
    info_msg1.textContent = "This is the menu builder live editor prototype version. Before using, please make sure to backup your plugins.js file in your project.";
    info_div.appendChild(info_msg1);
    const info_msg2 = document.createElement('p');
    info_msg2.id = "Editor_Msg";
    info_msg2.textContent = "Menu Setup will allow you to setup the various menus/scenes (More accurately scenes) to a (currently) limited extent.";
    info_div.appendChild(info_msg2);
    const info_msg3 = document.createElement('p');
    info_msg3.id = "Editor_Msg";
    info_msg3.textContent = "Override Setup will allow you to set which scenes should be overriden by which menu/scene configuration. It allows you to replace default RPG Maker scenes with your own custom menu.";
    info_div.appendChild(info_msg3);
    const info_msg4 = document.createElement('p');
    info_msg4.id = "Editor_Msg";
    info_msg4.textContent = "The RPG Maker Editor must be closed before any changes to the menu are applied properly. Leaving it open will only make changes temporary.";
    info_div.appendChild(info_msg4);
}

const refresh_plugin = function(){}

const modifyOverrides = function(override_data, reset_editor){
    const source = window.opener;
    const string_override_data = JSON.stringify(
        override_data.map((data)=>{
            return JSON.stringify(data);
        })
    );
    const plugins = source.$plugins;
    const plugin_data = plugins.find((plugin)=>{
        return plugin.name === "Synrec_MenuBuilder";
    });
    parameters = plugin_data.parameters;
    parameters['Scene Overrides'] = string_override_data;
    source.RESET_MENU_BUILDER(plugin_data, null);
    reloadOverrideContents(plugin_data);
    if(reset_editor){
        load_editor();
    }
}

const recompileMenus = function(menu_data, scene_data, reset){
    const source = window.opener;
    const string_menu_data = JSON.stringify(
        menu_data.map((menu)=>{
            menu['Preload Backgrounds'] = JSON.stringify(menu['Preload Backgrounds']);
            menu['On Load Script Calls'] = JSON.stringify(
                (menu['On Load Script Calls'] || []).map((script)=>{
                    return JSON.stringify(script);
                })
            )
            menu['Backgrounds'] = JSON.stringify(
                (menu['Backgrounds'] || []).map((bg_config)=>{
                    return JSON.stringify(bg_config);
                })
            )
            menu['Back Graphics'] = JSON.stringify(
                (menu['Back Graphics'] || []).map((bg_config)=>{
                    return JSON.stringify(bg_config);
                })
            )
            const stringify_dimension_config = function(config){
                config['X'] = config['X'].toString();
                config['Y'] = config['Y'].toString();
                config['Width'] = config['Width'].toString();
                config['Height'] = config['Height'].toString();
                return JSON.stringify(config);
            }
            const stringify_style_config = function(config){
                config['Font Size'] = config['Font Size'].toString();
                config['Font Outline Thickness'] = config['Font Outline Thickness'].toString();
                config['Window Opacity'] = config['Window Opacity'].toString();
                config['Show Window Dimmer'] = config['Show Window Dimmer'].toString();
                return JSON.stringify(config);
            }
            const stringify_selc_req = function(req){
                if(!req){
                    const obj = {};
                    obj['Variable Requirements'] = "[]";
                    obj['Switch Requirements'] = "[]";
                    obj['Item Requirements'] = "[]";
                    obj['Weapon Requirements'] = "[]" ;
                    obj['Armor Requirements'] = "[]";
                    return JSON.stringify(obj);
                }
                req['Variable Requirements'] = JSON.stringify(
                    (req['Variable Requirements'] || []).map((var_req)=>{
                        return JSON.stringify(var_req);
                    })
                )
                req['Switch Requirements'] = JSON.stringify(req['Switch Requirements']);
                req['Item Requirements'] = JSON.stringify(
                    (req['Item Requirements'] || []).map((itm_req)=>{
                        return JSON.stringify(itm_req);
                    })
                )
                req['Weapon Requirements'] = JSON.stringify(
                    (req['Weapon Requirements'] || []).map((wep_req)=>{
                        return JSON.stringify(wep_req);
                    })
                )
                req['Armor Requirements'] = JSON.stringify(
                    (req['Armor Requirements'] || []).map((arm_req)=>{
                        return JSON.stringify(arm_req);
                    })
                )
                return JSON.stringify(req);
            }
            const stringify_selc_btn = function(btn){
                if(!btn)return "";
                btn['Cold Graphic'] = JSON.stringify(btn['Cold Graphic']);
                btn['Hot Graphic'] = JSON.stringify(btn['Hot Graphic']);
                return JSON.stringify(btn);
            }
            const stringify_selc_optn = function(option){
                if(!option)return "";
                option['Display Requirements'] = stringify_selc_req(option['Display Requirements']);
                option['Select Requirements'] = stringify_selc_req(option['Select Requirements']);
                option['Static Graphic'] = JSON.stringify(option['Static Graphic']);
                option['Animated Graphic'] = JSON.stringify(option['Animated Graphic']);
                option['Pictures'] = JSON.stringify(option['Pictures']);
                option['Scene Button'] = stringify_selc_btn(option['Scene Button']);
                option['Description'] = JSON.stringify(option['Description']);
                option['Code Execution'] = JSON.stringify(option['Code Execution']);
                return JSON.stringify(option);
            }
            const stringify_selc_window = function(obj){
                obj['Gauges'] = JSON.stringify(
                    (obj['Gauges'] || []).map((gauge)=>{
                        return JSON.stringify(gauge);
                    })
                )
                obj['Selection Options'] = JSON.stringify(
                    (obj['Selection Options'] || []).map((option)=>{
                        return stringify_selc_optn(option);
                    })
                )
                obj['Window Font and Style Configuration'] = stringify_style_config(obj['Window Font and Style Configuration']);
                obj['Dimension Configuration'] = stringify_dimension_config(obj['Dimension Configuration']);
                return JSON.stringify(obj);
            }
            menu['Update Codes'] = JSON.stringify(
                menu['Update Codes'].map((code)=>{
                    return JSON.stringify(code);
                })
            )
            menu['Selection Window'] = stringify_selc_window(menu['Selection Window']);
            const stringify_actor_window = function(obj){
                obj['Gauges'] = JSON.stringify(
                    (obj['Gauges'] || []).map((gauge)=>{
                        return JSON.stringify(gauge);
                    })
                )
                obj['Draw Base Params'] = JSON.stringify(
                    (obj['Draw Base Params'] || []).map((param)=>{
                        return JSON.stringify(param);
                    })
                )
                obj['Draw Ex Params'] = JSON.stringify(
                    (obj['Draw Ex Params'] || []).map((param)=>{
                        return JSON.stringify(param);
                    })
                )
                obj['Draw Sp Params'] = JSON.stringify(
                    (obj['Draw Sp Params'] || []).map((param)=>{
                        return JSON.stringify(param);
                    })
                )
                obj['Window Font and Style Configuration'] = stringify_style_config(obj['Window Font and Style Configuration']);
                obj['Dimension Configuration'] = stringify_dimension_config(obj['Dimension Configuration']);
                return JSON.stringify(obj);
            }
            menu['Actor Selection Window'] = stringify_actor_window(menu['Actor Selection Window']);
            menu['Actor Data Windows'] = JSON.stringify(
                (menu['Actor Data Windows'] || []).map((window)=>{
                    return stringify_actor_window(window);
                })
            )
            const stringify_selc_data_window = function(obj){
                obj['Gauges'] = JSON.stringify(
                    (obj['Gauges'] || []).map((gauge)=>{
                        return JSON.stringify(gauge);
                    })
                )
                obj['Window Font and Style Configuration'] = stringify_style_config(obj['Window Font and Style Configuration']);
                obj['Dimension Configuration'] = stringify_dimension_config(obj['Dimension Configuration']);
                return JSON.stringify(obj);
            }
            menu['Selection Data Windows'] = JSON.stringify(
                (menu['Selection Data Windows'] || []).map((window)=>{
                    return stringify_selc_data_window(window);
                })
            )
            const stringify_disp_req = function(req){
                if(!req){
                    const obj = {};
                    obj['Game Switch'] = "0";
                    obj['Game Variable'] = "0";
                    obj['Variable Maximum'] = "0";
                    obj['Variable Maximum'] = "0" ;
                    obj['Code'] = "";
                    return JSON.stringify(obj);
                }else{
                    return JSON.stringify(req);
                }
            }
            const stringify_basic_window = function(obj){
                obj['Draw Texts'] = JSON.stringify(
                    (obj['Draw Texts'] || []).map((text_data)=>{
                        text_data['Text'] = JSON.stringify(text_data['Text']);
                        return JSON.stringify(text_data);
                    })
                )
                obj['Text References'] = JSON.stringify(obj['Text References']);
                obj['Draw Pictures'] = JSON.stringify(
                    (obj['Draw Pictures'] || []).map((config)=>{
                        return JSON.stringify(config);
                    })
                )
                obj['Gauges'] = JSON.stringify(
                    (obj['Gauges'] || []).map((gauge)=>{
                        return JSON.stringify(gauge);
                    })
                )
                obj['Display Requirements'] = stringify_disp_req(obj['Display Requirements']);
                obj['Window Font and Style Configuration'] = stringify_style_config(obj['Window Font and Style Configuration']);
                obj['Dimension Configuration'] = stringify_dimension_config(obj['Dimension Configuration']);
                console.log(JSON.stringify(obj));
                return JSON.stringify(obj);
            }
            menu['Basic Windows'] = JSON.stringify(
                (menu['Basic Windows'] || []).map((window)=>{
                    return stringify_basic_window(window)
                })
            )
            menu['Foregrounds'] = JSON.stringify(
                (menu['Foregrounds'] || []).map((bg_config)=>{
                    return JSON.stringify(bg_config);
                })
            )
            menu['Fore Graphics'] = JSON.stringify(
                (menu['Fore Graphics'] || []).map((bg_config)=>{
                    return JSON.stringify(bg_config);
                })
            )
            return JSON.stringify(menu);
        })
    )
    const plugins = source.$plugins;
    const plugin_data = plugins.find((plugin)=>{
        return plugin.name === "Synrec_MenuBuilder";
    });
    parameters = plugin_data.parameters;
    parameters['Menu Configurations'] = string_menu_data;
    source.RESET_MENU_BUILDER(plugin_data, scene_data);
    if(reset){
        load_editor();
        createMenuListContents();
    }else{
        reloadMenuListContents(plugin_data);
    }
}