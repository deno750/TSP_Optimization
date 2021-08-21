
//GLOBAL variables
var tsp_instance="att48.tsp";
var tsp_method="TABU_LIN";
var time_limit=100;
var seed=123;

//DOM objects
var btn_solve = document.getElementById('btn_solve');
var div_solution=document.getElementById('solution');
var sel_instance=document.getElementById('tsp_instance');
var sel_method=document.getElementById('tsp_method');
var txt_timelimit=document.getElementById('time_limit');
var txt_seed=document.getElementById('seed');

function reqListener () {
  console.log(this.responseText);
}

//When button solve is clicked
function btn_solve_click(evt){
  //Check if the input is correct
  if (! parseInt(txt_timelimit.value) || ! parseInt(txt_seed.value)){
    alert("Some values are not integers");
    return;
  }

  //Get values from users
  tsp_instance=sel_instance.value;
  tsp_method=sel_method.value;
  time_limit=parseInt(txt_timelimit.value);
  seed=parseInt(txt_seed.value);

  console.log(tsp_instance);
  console.log(tsp_method);
  console.log(time_limit);
  console.log(seed);

  //Disable button to prevent user click more than once
  btn_solve.classList.add("disabled");

  
  
  /*??????????????????????????
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      // Typical action to be performed when the document is ready:
      document.getElementById("solution").innerHTML = xhttp.responseText;
    }
  };
  xhttp.open("GET", "http://www.example.org/example.txt", true);
  xhttp.send();*/

}


// Setup button listener.
btn_solve.addEventListener('click',btn_solve_click);