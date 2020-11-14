/*
We store our game status element here to allow us to more easily 
use it later on 
*/
let gameActive = true;

var row=0;
var col=0;
all_boards = {"white": [], "black": []};
var players = [["white", "O"], ["black", "X"]];
var act_board_index = -1;

function handleCellClick(clickedCellEvent) {
    // === Get clicked cell index ===
    const clickedCell = clickedCellEvent.target;
    if (clickedCell.innerHTML != "" || !gameActive) {
        return;
    }
    console.log("Clicked "+clickedCell.getAttribute('row')+" "+clickedCell.getAttribute('col'));

    // === Send move and wait for response ===
    var socket = io(namespace);
    socket.emit("move", {"row":clickedCell.getAttribute('row'), 
                         "col":clickedCell.getAttribute('col')});
    socket.on('update_nodes', handleMove);
}

function color_moves(moves, color){
    for(i in moves){
        document.getElementById("cell_"+moves[i]).style.background = "#fffb00";
    }
}

function display_board(index){
    document.querySelectorAll('.cell')
        .forEach(cell => cell.innerHTML = "");
    for(p of players){
        var player = p[0];
        var note = p[1];
        for(step of all_boards[player][index]){
            document.getElementById("cell_"+step).innerHTML = note;
        }
    }
}

function handleMove(message){
    console.log("Update nodes");
    var data = JSON.parse(message["data"]);
    console.log("Received data from socket: ", data);
    if(message["end"]=="true"){
        gameActive = false;
    }

    for(p of players){
        var player = p[0];
        var note = p[1];
        for(board of data[player]){
            all_boards[player].push(board);
        }
        console.log("Last board: (",player,")", data[player][data[player].length-1]);
    }
    act_board_index = all_boards[players[0][0]].length-1;
    display_board(act_board_index);
    console.log("Update nodes end");
}

function handleRestartGame() {
    gameActive = true;
    all_boards = {"white": [], "black": []};
    act_board_index = -1;
    document.querySelectorAll('.cell')
               .forEach(cell => cell.innerHTML = "");
    
    var socket = io('/test');
    socket.emit("build", {"COL":col});
    socket.on('update_nodes', handleMove);
}

function create_board(limits){
    mylist = limits.split("x");
    row = parseInt(mylist[0]);
    col = parseInt(mylist[1]);

    var socket = io('/test');
    socket.emit("build", {"COL":col});
    socket.on('update_nodes', handleMove);

    var board = document.getElementById("board");
    board.style.setProperty("--col", col);
    board.style.setProperty("--row", row);
    board.innerHTML = "";
    board.row = row;
    board.col = col;

    var sum = 0;
    for(j=0;j<row;j++){
        for(i=0;i<col;i++){
            var div = document.createElement('div');
            var id = i*row+j;
            div.setAttribute("index", id);
            div.setAttribute("id", "cell_"+id);
            div.setAttribute("row", j);
            div.setAttribute("col", i);
            //div.innerHTML = id;

            div.className="cell";
            div.addEventListener('click', handleCellClick);
            board.appendChild(div);
            sum++;
        }
    }
}

// ===============================
//           LISTENERS
// ===============================
window.onload = function () { 
    document.getElementById("board_size").selectedIndex=0;
}

switch_counter = 0;
document.querySelector('.switch_players').addEventListener('click', function(){
    var p1 = document.getElementById("switch1").innerText;
    var p2 = document.getElementById("switch2").innerText;

    document.getElementById("switch1").innerText = p2;
    document.getElementById("switch2").innerText = p1;
    var warning_div = document.getElementById("last_warning");
    
    if(switch_counter == 0){
        warning_div.innerHTML = "Nie try! ;)";
    }
    else if(switch_counter == 1){
        warning_div.innerHTML = "Don't do that again!";
    }
    else if(switch_counter == 2){
        warning_div.innerHTML = "I will crash, last warning!!!";
    }
    else if(switch_counter == 3){
        warning_div.innerHTML = "";
        warning_div.style.height = "400px";
        warning_div.style.width = "600px";
        warning_div.style.backgroundImage = "url('https://lh3.googleusercontent.com/proxy/IDU2nYcnETUCY276aipoJyOTiOtlSzG93MUceBpL6YgntctCQaYOJkKZKYmijcMt5S1SVYrPhzosDwQrBIDZdxXN4IWUazVEgehrwYRTJif7Neb0C2AxN_BbXRfYVo5a1r_KbypD7u54THl1tXryow8xlD-whAih6zk')"; 
        warning_div.style.backgroundSize = "600px 400px";
    }
    switch_counter += 1;
});

document.getElementById('back_button').addEventListener('click', function(){
    if(act_board_index > 0){
        act_board_index -=1;
    }
    display_board(act_board_index);
});

document.getElementById('forward_button').addEventListener('click', function(){
    if(act_board_index < all_boards[players[0][0]].length-1){
        act_board_index +=1;
    }
    display_board(act_board_index);
});

document.querySelectorAll('.cell').forEach(cell => cell.addEventListener('click', handleCellClick));

document.querySelector('.game--restart').addEventListener('click', handleRestartGame);

