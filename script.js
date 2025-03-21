let roles = [];
let currentPlayer = 0;

function startGame() {
    const playerCount = document.getElementById("playerCount").value;
    if (playerCount <= 0) {
        alert("Vui lòng nhập số lượng hợp lệ!");
        return;
    }

    fetch(`/assign-roles?count=${playerCount}`)
        .then(response => response.json())
        .then(data => {
            roles = data.roles;
            currentPlayer = 0;
            document.getElementById("gameStatus").innerText = `Có ${playerCount} người chơi. Nhấn Random để xem từng người.`;
            document.getElementById("randomButton").disabled = false;

            const roleList = document.getElementById("roleList");
            roleList.innerHTML = ""; // Xóa danh sách cũ nếu có

            for (let i = 0; i < playerCount; i++) {
                const li = document.createElement("li");
                li.innerText = `Người chơi ${i + 1}: [Chưa tiết lộ]`;
                roleList.appendChild(li);
            }
        })
        .catch(error => console.error("Lỗi khi lấy vai trò:", error));
}

function randomRole() {
    if (currentPlayer < roles.length) {
        const roleList = document.getElementById("roleList");
        roleList.children[currentPlayer].innerText = `Người chơi ${currentPlayer + 1}: ${roles[currentPlayer]}`;
        currentPlayer++;
    }

    if (currentPlayer >= roles.length) {
        document.getElementById("randomButton").disabled = true;
        document.getElementById("gameStatus").innerText = "Đã tiết lộ tất cả vai trò!";
    }
}