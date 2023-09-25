function showMainBlock() {
    let obj = document.getElementById("main_block");
    obj.classList.remove("hidden-main-item");
    obj.style.display = "flex";
}

function hideAuthorizationBlock() {
    let obj = document.getElementById("authorization_block");
    obj.classList.add("hidden-form-item");
    window.setTimeout(
        function removethis() {
            obj.style.display='none';
        },
    300);
    showMainBlock();
}

function hideRegistrationBlock() {
    let obj = document.getElementById("registration_block");
    obj.classList.add("hidden-form-item");
    window.setTimeout(
        function removethis() {
            obj.style.display='none';
        },
    300);
    showMainBlock();
}

function showRegistrationBlock() {
    let obj = document.getElementById("main_block");
    obj.classList.add("hidden-main-item");
    window.setTimeout(
        function removethis() {
            obj.style.display='none';
        },
    300);
    let reg = document.getElementById("registration_block");
    reg.classList.remove("hidden-form-item");
    reg.style.display='block';
}

function showAuthorizationBlock() {
    let obj = document.getElementById("main_block");
    obj.classList.add("hidden-main-item");
    window.setTimeout(
        function removethis() {
            obj.style.display='none';
        },
    300);
    let reg = document.getElementById("authorization_block");
    reg.classList.remove("hidden-form-item");
    reg.style.display='block';
}

function clearActiveNote() {
    let projects = document.getElementById("created_notes").getElementsByTagName("button");
    Array.from(projects).forEach((el) => el.classList.remove("active"));
}

function setActiveNote(el) {
    clearActiveNote();
    el.target.classList.add("active");
    document.getElementById("create_new_note").name = el.target.name
    document.getElementById("note_inner_content").style.display = "block";
    document.getElementById("update_note_text").name = el.target.name;
    document.getElementById("note_text_content").innerHTML = el.target.title;
}

/*
function closeCurrentProject(el) {
    clearActiveProject();
    document.getElementById("create_box").style.display = "block";
    document.getElementById("project_content").style.display = "none";

}
*/

function hasChildProject(parent, name) {
    arr = new Array;
    for (let i of parent.childNodes) {
        if (i.name == name) {
            return true;
        }
    }
    return false;
}

function pushButtonToDOM(parent, text, project, content="") {
    let item = document.createElement("button");
    let maxLen = 15;
    if (project) {
        item.addEventListener("click", setActiveProject);
    } else {
        item.addEventListener("click", setActiveNote);
        item.title = content;
    }
    item.textContent = text.length > maxLen?
        text.substr(0, maxLen - 3) + "...":
        text;
    item.name = text
    parent.appendChild(item);

}

function removeAllChidren(el) {
    while (el.lastElementChild) {
        el.removeChild(el.lastElementChild);
    }
}

function setActiveProject(el) {
    document.getElementById("create_box").style.display = "none";
    document.getElementById("note_content").style.display = "block";
    document.getElementById("create_new_note").name = el.target.name;
    updateCreatedNotes(el.target.name);
}

function closeCurrentNote(el) {
    document.getElementById("create_box").style.display = "block";
    document.getElementById("note_content").style.display = "none";
    document.getElementById("note_inner_content").style.display = "none";
    removeAllChidren(document.getElementById("created_notes"));
}








