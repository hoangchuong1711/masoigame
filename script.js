console.log("Script đã chạy!");

function joinGame() {
    const playerName = document.getElementById("playerName").value;
    const roomID = document.getElementById("roomID").value;
    
    fetch("/join-game", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ name: playerName, roomID: roomID })
    });

    ws = new WebSocket(`${location.protocol === "https:" ? "wss" : "ws"}://${location.host}/ws/${roomID}`);

    ws.onmessage = (event) => {
        console.log("Từ server:", event.data);
        const li = document.createElement("li");
        li.textContent = event.data;
        document.getElementById("playerList").appendChild(li);
    };
}

function startGame() {
    const roomID = document.getElementById("roomID").value;
    fetch("/start-game", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ roomID: roomID })
    });
}
