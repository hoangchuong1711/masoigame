function randomRole() {
    fetch('/random-role')
    .then(response => response.text())
    .then(data => document.getElementById('role').innerText = data);
}