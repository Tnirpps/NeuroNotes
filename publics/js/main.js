
const EMAIL_REGEXP = /^(([^<>()[\].,;:\s@"]+(\.[^<>()[\].,;:\s@"]+)*)|(".+"))@(([^<>()[\].,;:\s@"]+\.)+[^<>()[\].,;:\s@"]{2,})$/iu;

function onInputEmail(el) {
    if (!isEmailValid(el.value)) {
        el.style.borderColor = 'red';
        return;
    }
    el.style.borderColor = '';
}

function isEmailValid(value) {
    return (EMAIL_REGEXP.test(value));
}

function onInputPass(el) {
    if (!isPassValid(el.value)) {
        el.style.borderColor = 'red';
        return;
    }
    el.style.borderColor = '';
}

function isPassValid(value) {
    return (value.length >= 8);
}

function onInputName(el) {
    if (!isNameValid(el.value)) {
        el.style.borderColor = 'red';
        return;
    }
    el.style.borderColor = '';
}

function isNameValid(value) {
    return (value.length > 0);
}






function validateRegForm() {
    let name  = document.getElementById("reg_user").value;
    let email = document.getElementById("reg_email").value;
    let pass  = document.getElementById("reg_psw").value;
    return (isNameValid(name) && isPassValid(pass) && isEmailValid(email));
}




