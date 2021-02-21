'use strict';
// this is an URL for local testing
// adopt the IP to your situation
const myURL = 'http://192.168.0.140/api/getMeshStructure';
const width = 800;
const height = 600;
const interval = 10000;

// this is the area to paint on
const svg = d3.select("#robonet");
svg.style('width', width);
svg.style('height', height);

let url = '/api/getMeshStructure';
let oldData;
/**
 * the method to fetch data
 */
const fetcher = () => {
	fetch(url)
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
				if(interval > 0)
					setTimeout(fetcher, interval);
			});
		} else if(response.status == '404') {
			url = myURL;
			fetcher()
		}
	});
}

fetcher();


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
		.force("center", d3.forceCenter(width / 2, height / 2));

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

