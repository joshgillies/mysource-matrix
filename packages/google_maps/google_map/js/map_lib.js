
	/**
	* This function is used to initialize the GMAP Object and set all the default parameters
	*
	*/
	function gmap_init()	{
		  var mapOptions = {
		    center: new google.maps.LatLng(centerLatitude, centerLongitude),
		    zoom: zoomLevel,
		    mapTypeId: map_type
		  };
		  map = new google.maps.Map(document.getElementById("map"), mapOptions);

	}//end gmap_init()


	/**
	* This function is used to add listener to the map for manipulation. Switch name is the name of the image/link for coloring.
	*
	*/
	function addListeners(name, switch_name)	{
		if (current_listener_name) {
			var tool_div = document.getElementById(current_listener_name);
			if (tool_div) {
				tool_div.style.borderColor	= 'white';
			}//end if
		}//end if
		if (current_listener) {
			google.maps.event.removeListener(current_listener);
			current_listener	= null;
		}//end if

		switch (name) {
			case 'lat_lng_mouse_location' :
				current_listener = google.maps.event.addListener(map, 'mousemove', function(event) {
					var  overlay = new google.maps.OverlayView();
					overlay.draw = function() {};
					overlay.setMap(map);
					var pixelLocation = overlay.getProjection().fromLatLngToDivPixel(event.latLng); 
				 	console.log('Latitude Longtitude:' + event.latLng + ' at PIXEL LOCATION:' + pixelLocation);
				 }//end function
				)//end addListener
				break;

			case 'zoom_change' :
				current_listener = google.maps.event.addListener(map, 'zoom_changed', function() {
				var zoomLevel = map.getZoom();
				      console.log('ZOOM CHANGED to ' + zoomLevel);
				     }//end function
				)//end addListener
				break;

			case 'add_marker' :
				current_listener = google.maps.event.addListener(map, 'click',
					function(event) {
						try {
							var street_view_enabled;
							if(typeof myPano != 'undefined') {
							    street_view_enabled = true;
							}
							else {
							
							    street_view_enabled = false;
							}
							var marker = addMarker(null, event.latLng.lat(), event.latLng.lng(), null, null, street_view_enabled);
							newMarkers.push(marker);
							current_marker	= marker;
						} catch (e) {}
					}//end function
				);

				break;
			case 'street_view':
				//We should decide whether we should allow set POV dynamically
				current_listener = google.maps.event.addListener(map,"click", function(event) {
					if (myPano) {
						myPano.setPosition(event.latLng);
					}//end if
					var div = document.getElementById("street_view");
					if (div && div.style.display !='block') {
						div.style.display = 'block';
					}//end if
				});
				break;
		}//end switch

		current_listener_name	= name+'_tool';
		var new_tool_div = document.getElementById(current_listener_name);
		if (new_tool_div) {
			new_tool_div.style.borderColor	= 'red';
		}//end if

		//return current_listener;

	}//end addListeners()



	/**
	* This function is used to add Input Form to add Marker
	*
	*/
	function addInputFormListener(html)
	{
		GEvent.addListener(map, "click",
	      function(overlay, latlng) {
	        var inputForm  = document.createElement("form");
	        inputForm.setAttribute("action", "");
	        //inputForm.onsubmit  = function() { storeMarker(); return false;};

	        var lng  = latlng.lng();
	        var lat  = latlng.lat();
	        inputForm.innerHTML = html;

	       map.openInfoWindow(latlng, inputForm);
	      }//end function
	    );//end addListener
	}


	/**
	* This function is used to add a marker on to the map
	*
	*/
	function addMarker(name, latitude, longitude, icon_image, description, street_view_enabled) {
		if (icon_image != null && icon_image != '') {
			var icon_url_final = icon_image;
		} else {
			var icon_url_final = icon_url;
		}//end if
		if (icon_url_final != '' && icon_url_final != null) {
			 var image = new google.maps.MarkerImage(icon_url_final,
			    // This marker is 20 pixels wide by 32 pixels tall.
			    new google.maps.Size(icon_width, icon_height),
			    // The origin for this image is 0,0.
			    new google.maps.Point(0,0),
			    // The anchor for this image is the base of the flagpole at 0,32.
			    new google.maps.Point(icon_width/2, icon_height));

			    var myLatlng = new google.maps.LatLng(latitude, longitude);
			    var marker  = new google.maps.Marker({
				position : myLatlng, 
				map: map, 
				title : name,
				icon : image
			    });
		} else {
		 
			var myLatlng = new google.maps.LatLng(latitude, longitude);
			var marker  = new google.maps.Marker({
			    position : myLatlng, 
			    map: map, 
			    title : name
			});
		}//end else

		
		
		var infowindow = new google.maps.InfoWindow({
		    content: description
		});
		
		if (street_view_enabled) {
			google.maps.event.addListener(marker, "click", function() {
				current_marker = marker;
				if(description != null) {
				    infowindow.open(map,marker);
				}
				var latlng = marker.getPosition();
				var div = document.getElementById("street_view");
				if (div && div.style.display !='block') {
					div.style.display = 'block';
				}//end if
				if (myPano) {
					myPano.setPosition(latlng);
				}

			});
		} else {
		    
			google.maps.event.addListener(marker, 'click',
			  function () {
			    current_marker = marker;
			    if(description != null) {
				infowindow.open(map,marker);
			    }
			  }//end function
			)//end addListener
		}//end else


		return marker;

	}//end addMarker()


	

	/**
	* This function is used to set the map type
	*
	*/
	function setMapType(mapType)
	{
		map.setMapTypeId(mapType);
	}//end setMapType




	/**
	* This function is used to get the information about a marker
	*
	*/
	function getInfo(i)
	{
		google.maps.event.trigger(allMarkerList[i], "click");
	}//end getInfo()


	/**
	* This function is used to clear all the new markers from the map
	*
	*/
	function clearNewMarker(marker_array)
	{
		for (var i =0; i < marker_array.length; i++) {
			if (marker_array[i]) {
				marker_array[i].setMap(null);
				marker_array[i] = null;
			}
		}//end for
	}//end clearNewMarker()


	/**
	* This function is used to get the nearest marker to the current marker (passed in).
	*
	*/
	function getClosestLocationForMarker(marker)
	{
		var current_latlng	= marker.getPosition();
		var current_lat		= current_latlng.lat();
		var current_lng		= current_latlng.lng();

		var closest_distance	= 0;
		var closest_marker		= null;

		for (var obj in allMarkerList) {
			if (!allMarkerList[obj].visible) {
				var latlng	= allMarkerList[obj].getPosition();
				var lng		= latlng.lng();
				var lat		= latlng.lat();

				// Haversine formular
				var R = 6371; // km
				var dLat = (current_lat-lat)* Math.PI / 180;
				var dLon = (current_lng-lng)* Math.PI / 180;
				var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
				        Math.cos(current_lat* Math.PI / 180) * Math.cos(lat* Math.PI / 180) *
				        Math.sin(dLon/2) * Math.sin(dLon/2);
				var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
				var distance = R * c;

				if ((distance < closest_distance || closest_distance == 0) && distance != 0) {
					closest_distance	= distance;
					closest_marker		= allMarkerList[obj];
				}//end if
			}//end if
		}//end for

		return closest_marker;

	}//end getClosestLocationForMarker()


	/**
	* This function is used to draw a line between two markers
	*/
	function drawLineBetweenMarkers(coordinate_1, coordinate_2, color)
	{
		var path = [coordinate_1, coordinate_2];
		var polyline = new google.maps.Polyline({
		    path : path,
		    strokeColor : color,
		    strokeWeight: 2   
		});
		newPolylines.push(polyline);
		polyline.setMap(map);
	}//end drawLineBetweenMarkers()


	
	/**
	* Make a element disappear or appear (mostly used for divs)
	*/
	function toggleDiv(div_name) {
		var div = document.getElementById(div_name);
		if (div.style.display!='none') {
			div.style.display='none';
		} else {
			div.style.display='block';
		}//end if
	}//end toggleDiv()


	/**
	*
	*/
	function findLocationFromAddress(address, extra_text, icon_image, street_view_enabled) {
		if (address) {
		    if (!extra_text)  {
			extra_text = '';
		    }
		    else {
			extra_text += ' ';
		    }
		    geocoder.geocode( { 'address': address}, function(results, status) {
		    if (status == google.maps.GeocoderStatus.OK) {
			map.setCenter(results[0].geometry.location);
			var marker = addMarker(address, results[0].geometry.location.lat(), results[0].geometry.location.lng(), null, extra_text+address, street_view_enabled);
			newMarkers.push(marker);
			updateAddressList(marker, address);
			current_marker	= marker;
		    } else {
			alert("Geocode was not successful for the following reason: " + status);
		    }
		    });
		}//end if address

	}//end showAddress()


	/**
	* Update the address list on the map (could be hidden, but update the list anyway)
	*/
	function updateAddressList(marker, address)
	{
		var newLocation	= {};
		newLocation['markerObj']	= marker;
		newLocation['address']		= address;
		addressList.push(newLocation);

		var address_list_div = document.getElementById('new_address_list');
		if (address_list_div) {
			var new_text_ele	= document.createTextNode(' '+address);
			var new_break		= document.createElement('br');
			var new_link		= document.createElement('a');

			var	addressListIndex	= addressList.length - 1;
			new_link.setAttribute('href', 'javascript:populateAssetBuilderForm(addressList['+addressListIndex+'][\'markerObj\'], addressList['+addressListIndex+'][\'address\']);');
			var new_text_link	= document.createTextNode('[-]');
			new_link.appendChild(new_text_link)

			address_list_div.appendChild(new_link);
			address_list_div.appendChild(new_text_ele);
			address_list_div.appendChild(new_break);
		}//end if
	}//end udpateAddressList()


	/**
	* If we have an asset builder form on the map, clicking on the address in the new address list will populate the form
	*/
	function populateAssetBuilderForm(marker, address)
	{

		document.getElementById('asset_builder').style.display = 'block';

		var	latlng	= marker.getPosition();
		populateElementValue(FormEle.longitude, latlng.lng());
		populateElementValue(FormEle.latitude, latlng.lat());
		populateElementValue(FormEle.description, address);

	}//end if


	/**
	* Change the value/innerHTML of a element
	*/
	function populateElementValue(id, content)
	{
		var element	= document.getElementById(id);
		if (element) {
			element.value	= content;
			element.innerHTML	= content;
			return true;
		}//end if
		return false;

	}//end populateAssetBuilderForm()


	/**
	* If we are using GMAP to look up address in LDAP, this function is used to cache the request everytime an address is found
	*/
	function updateMapCache(cache_home_url, cache_key, uid, lat, lng)
	{
		var	xmlHttp;
		try {
			xmlHttp	= new XMLHttpRequest();
		} catch (e) {
			try {
				xmlHttp= new ActiveXObject("Msxml2.XMLHTTP");
			} catch (e) {
				try {
					xmlHttp	= new ActiveXObject("Microsoft.XMLHTTP");
				} catch (e) {
					alert("Your browser does not support AJAX");
					return false;
				}//end catch
			}//end catch
		}//end catch
		xmlHttp.onreadystatechange=function()
		{
			if (xmlHttp.readyState==4) {

			}
		}
		xmlHttp.open("GET", cache_home_url+'?cache_key='+cache_key+'&uid='+uid+'&lat='+lat+'&lng='+lng, true);
		xmlHttp.send(null);

	}//end updateMapCache()


	/**
	*
	*
	*/
	function toggleDisplay(marker_array, key_index)
	{
		if (!marker_array[key_index].toggle) {
			for (var obj in marker_array[key_index]) {
				if (obj != 'toggle') {
					marker_array[key_index][obj].setMap(map);
				}//end if
			}//end for
		} else {
			for (var obj in marker_array[key_index]) {
				if (obj != 'toggle') {
					marker_array[key_index][obj].setMap(null);
				}//end if
			}//end for
		}
		marker_array[key_index].toggle = (!marker_array[key_index].toggle);
	}//end toggleDisplay


	/**
	*
	*
	*/
	function getClosestLocation(color)
	{
		var last_marker		= getLastMarker();
		// Only execute this when there is a last selected marker
		if (last_marker) {
			var closest_marker	= getClosestLocationForMarker(last_marker);
			if (!color)  {
				color	= '#ff0000';
			}//end if
			drawLineBetweenMarkers(last_marker.getPosition(), closest_marker.getPosition(), color);
		}//end if

	}//end getClosestLocation()


	/**
	*
	*
	*/
	function getLastMarker()
	{
		if (!current_marker) {
			var num_markers		= newMarkers.length;
			var	last_marker		= newMarkers[num_markers-1];
			return last_marker;
		} else {
			return current_marker;
		}//end else
	}//end getLastMarker()


	/**
	* Javascript does not have a sleep/wait function so we use this (very CPU intensive though - so avoid it)
	* setTimeout does not work as we need, it will execute the following function immediately
	*/
	function wait(msecs)
	{
		var start	= new Date().getTime();
		var cur		= start
		while(cur-start < msecs)
		{
			cur	= new Date().getTime();
		}//end while

	}//end wait()
