
const EMAIL_REGEXP = /^(([^<>()[\].,;:\s@"]+(\.[^<>()[\].,;:\s@"]+)*)|(".+"))@(([^<>()[\].,;:\s@"]+\.)+[^<>()[\].,;:\s@"]{2,})$/iu;

function onInputEmail(el) {
  if (isEmailValid(el.value) || el.value.length == 0) {
    el.style.borderColor = 'green';
  } else {
    el.style.borderColor = 'red';
  }
}

function onInputPass(el) {
  if (isPassValid(el.value)) {
    el.style.borderColor = 'green';
  } else {
    el.style.borderColor = 'red';
  }
}

function isEmailValid(value) {
    return EMAIL_REGEXP.test(value);
}

function isPassValid(value) {
    return (value.length >= 8 || value.length == 0);
}

