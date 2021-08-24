
//GLOBAL variables
var tsp_instance="att48.tsp";
var tsp_method="GREEDY";
var time_limit=100;
var seed=123;

//DOM objects
var btn_solve = document.getElementById('btn_solve');
var div_solution=document.getElementById('solution');
var sel_instance=document.getElementById('tsp_instance');
var sel_method=document.getElementById('tsp_method');
var txt_timelimit=document.getElementById('time_limit');
var txt_seed=document.getElementById('seed');
var div_sol_plot=document.getElementById('solution_plot');
var div_cost_plot=document.getElementById('cost_iteration_plot');
var loadingModal = new bootstrap.Modal(document.getElementById('loadingModal'), {
  backdrop: 'static', // to prevent closing for user clicking
  keyboard: false
});


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

  //Show the spinner
  loadingModal.show();
  
  //POST REQUEST
  var xhttp = new XMLHttpRequest();
  /*xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      // Write the output
      div_solution.innerHTML = xhttp.responseText;
      
      //Get the solution image
      div_sol_plot.innerHTML = "";
      div_sol_plot.innerHTML ="<img src='"+server_IP+"/get_image?instance="+tsp_instance+"' class='img-fluid'/>"

      //Get the cost iteration image
      //TODO...

      //Enable button again
      btn_solve.classList.remove("disabled");

      //Remove the loading spinner
      loadingModal.hide();
      
    }
  };*/
  console.log(server_IP+"/compute");
  xhttp.onload = function(e) {
    
    // Write the output
    //div_solution.innerHTML = xhttp.responseText;
      
    //Get the solution image (the random number is used because the browser save in the cache the image and don't make the API call)
    div_sol_plot.innerHTML ="<img src='"+server_IP+"/get_image?instance="+tsp_instance+"&a="+Math.floor(Math.random() * (2**128)).toString()+"' class='img-fluid'/>";

    //Get the cost iteration image
    //TODO...

    //Enable button again
    btn_solve.classList.remove("disabled");

    //Remove the loading spinner
    loadingModal.hide();

    console.log("hi");
  };
  xhttp.open("POST", server_IP+"/compute", true);
  xhttp.setRequestHeader("Userid","stefano");
  xhttp.setRequestHeader("Instance",tsp_instance);
  xhttp.setRequestHeader("Method",tsp_method);
  xhttp.setRequestHeader("Time-Limit",time_limit.toString());
  xhttp.setRequestHeader("Seed",seed.toString());
  xhttp.send();

}


function sel_instance_change(evt){
  tsp_instance=sel_instance.value;
  div_sol_plot.innerHTML ="<img src='"+server_IP+"/get_instance_not_solved?instance="+tsp_instance+"' class='img-fluid'/>"

}

// Setup events listener.
btn_solve.addEventListener('click',btn_solve_click);  //when user click on the Optimize button
sel_instance.addEventListener("change",sel_instance_change);  //when user change the instance
