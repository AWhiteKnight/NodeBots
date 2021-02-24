'use strict';
// these are the polling interval values
const gamepadSendInterval = 100;
const meshFetchInterval =  1000;

// this is an URL for local testing
// adopt the IP to your situation
const getMeshURL = 'http://192.168.0.140/api/getMeshStructure';
const postMsgURL = 'http://192.168.0.140/api/postMessage';

// we need to inactivate CORS
const corsHeader = "Access-Control-Allow-Origin";
const corsValue = "*";

// definitions for the gamepad
const gamepadBox = document.getElementById("gamepad");
const gamepadWidth = gamepadBox.offsetWidth;
const gamepadHeight = gamepadBox.offsetHeight;

const joy1 = new JoyStick('joy1Div');
const joy2 = new JoyStick('joy2Div');

let vel, yaw, nick, roll, sum;
let msg, target = 2731577066;
let mode = 1;
let send = true;
let postMsgUrl = "/api/postMessage";

// the method to post msgs
const poster = () => {
	if (mode == 1 || mode == 2) 
	 	{  yaw = joy1.GetX(); roll = joy2.GetX(); }
	else
		{ roll = joy1.GetX();  yaw = joy2.GetX(); }
	
	if (mode == 1 || mode == 3) 
		{ nick = joy1.GetY(); vel = joy2.GetY(); }
	else
		{  vel = joy1.GetY(); nick = joy2.GetY(); }
	
	msg = {
		target: target,
		rc_ctrl: {
			vel: vel,
			yaw: yaw,
			nick: nick,
			roll: roll
		}
	};
	sum = Math.abs(vel) + Math.abs(yaw) + Math.abs(nick) + Math.abs(roll);

	// if sum of values is 0 we will do a last send
	if(send || sum != 0) {
		(sum == 0) ? send = false : send = true;
		fetch(postMsgUrl, {
			method: "POST",
			mode: 'no-cors',
			headers: {
				corsHeader: corsValue
			}, 
			body: (JSON.stringify(msg))
		}).then(response => {
			//console.log(response.status);
			if(response.status == '0' || response.status == '200') {
				setTimeout(poster, gamepadSendInterval);
			} else if(response.status == '405' || response.status == '404') {
				postMsgUrl = postMsgURL;
					setTimeout(poster, gamepadSendInterval);
			} else {
				console.log("error");
			}
		});
	}
	else
	{
		send = false;
		setTimeout(poster, gamepadSendInterval);
	}
};

// definitions for the mesh structure
const meshBox = document.getElementById("mesh");
const meshWidth = meshBox.offsetWidth;
const meshHeight = meshBox.offsetHeight;

let getMeshUrl = '/api/getMeshStructure';
let oldData;

// this is the area to paint on
const svg = d3.select("#robonet");

// the method to fetch mesh data
const fetcher = () => {
	fetch(getMeshUrl)
	.then(function(response) {
		//console.log(response);
		if(response.status == '200') {
			response.text()
			.then(function(text) {
				//console.log(text);
				if(oldData != text) {
					ellipseChart(JSON.parse(text));
					oldData = text;
				}
				setTimeout(fetcher, meshFetchInterval);
			});
		} else if(response.status == '404') {
			getMeshUrl = getMeshURL;
			setTimeout(fetcher, 100);
		}
	});
}

const ellipseChart = (data) => {

	// clear area
	//svg.selectAll("*").remove();
	svg.html(null);

	const root = d3.hierarchy(data, d => d.subs );
	const links = root.links();
	const nodes = root.descendants();

	for ( var i = 0; i < nodes.length; i++ ) {
		nodes[i].rx = 45; //nd.data.nodeId.length * 4.5; 
		nodes[i].ry = 12;
	} 

	const simulation = d3.forceSimulation()
		//.force("link", d3.forceLink(links).id(d => d.id).distance(0).strength(1))
		.force("link", d3.forceLink().id(d => d.data.nodeId))
		.force("collide", ellipseForce(6, 0.5, 5))
		.force("center", d3.forceCenter(meshWidth / 2, meshHeight / 2));

	const link = svg.append("g")
		.attr("class", "link")
		.selectAll("line")
		.data(links)
		.join("line");

	const node = svg.append("g")
		.attr("class", "node")
		.selectAll("ellipse").data(nodes).join("ellipse")
		.attr("rx", d => d.rx )
		.attr("ry", d => d.ry )
		.attr("fill", d => d.depth == 0 ? "#f6f" : d.data.root ? "#0ff" : "#0f0" )
		.attr("stroke", d => d.depth == 0 ? "#f0f" : d.data.root ? "#066" :"#0f0" )
		.call(drag(simulation));

	node.append("title")
		.text(d => d.data.nodeId);

	var text = svg.append("g")
		.attr("class", "labels")
		.selectAll("text").data(nodes).join("text")  
		.attr("dy", 3)
		.attr("text-anchor", "middle")
		.text( d => d.data.nodeId );

	simulation
		.nodes(nodes)
		.on("tick", ticked);

	simulation.force("link")
		.links(links);

	function ticked() {
		link
			.attr("x1", d => d.source.x)
			.attr("y1", d => d.source.y)
			.attr("x2", d => d.target.x)
			.attr("y2", d => d.target.y);

		node
			.attr("cx", d => d.x)
			.attr("cy", d => d.y);
		text
			.attr("x", d => d.x)
			.attr("y", d => d.y);
	}
};

const drag = simulation => {
  
	function dragstarted(event, d) {
	  if (!event.active) simulation.alphaTarget(0.3).restart();
	  d.fx = d.x;
	  d.fy = d.y;
	}
	
	function dragged(event, d) {
	  d.fx = event.x;
	  d.fy = event.y;
	}
	
	function dragended(event, d) {
	  if (!event.active) simulation.alphaTarget(0);
	  d.fx = null;
	  d.fy = null;
	}
	
	return d3.drag()
		.on("start", dragstarted)
		.on("drag", dragged)
		.on("end", dragended);
};

// start posting commands
poster();
// start fetching mesh structure
fetcher();

