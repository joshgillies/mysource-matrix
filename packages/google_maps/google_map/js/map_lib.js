
	/**
	* This function is used to initialize the GMAP Object and set all the default parameters
	*
	*/
	function gmap_init()	{
		if (GBrowserIsCompatible()) {
			map = new GMap2(document.getElementById("map"));
			//retrieveMarkers();
			var location = new GLatLng(centerLatitude, centerLongitude);
			map.setCenter(location, zoomLevel);
			map.setMapType(map_type);
	    }//end if

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
			GEvent.removeListener(current_listener);
			current_listener	= null;
		}//end if

		switch (name) {
			case 'lat_lng_mouse_location' :
				current_listener = GEvent.addListener(map, 'mousemove', function(latlng) {
					var pixelLocation  = map.fromLatLngToDivPixel(latlng);
				 	GLog.write('Latitude Longtitude:' + latlng + ' at PIXEL LOCATION:' + pixelLocation);
				 }//end function
				)//end addListener
				break;

			case 'zoom_change' :
				current_listener = GEvent.addListener(map, 'zoomend', function(oldLevel, newLevel) {
				      GLog.write('ZOOM CHANGED from '+ oldLevel + ' to ' + newLevel);
				     }//end function
				)//end addListener
				break;

			case 'add_marker' :
				current_listener = GEvent.addListener(map, 'click',
					function(overlay, latlng) {
						try {
							if (icon_url != '' && icon_url != null) {
								var icon = new GIcon();
								icon.image = icon_url;
								icon.iconSize = new GSize(icon_width, icon_height);
								icon.iconAnchor  = new GPoint(14, 25);
								icon.infoWindowAnchor = new GPoint(14, 14);
								var marker  = new GMarker(latlng, icon);
							} else {
								var marker  = new GMarker(latlng);
							}//end else
							newMarkers.push(marker);
							current_marker	= marker;
							map.addOverlay(marker);
						} catch (e) {}
					}//end function
				);

				break;
			case 'street_view':
				//We should decide whether we should allow set POV dynamically
				current_listener = GEvent.addListener(map,"click", function(overlay,latlng) {
					if (myPano) {
						myPano.setLocationAndPOV(latlng);
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
	* This function is used to turn mouse wheel scroll zooming on or off
	*
	*/
	function toggle_wheel_scrool()
	{
		var tool	= document.getElementById('mouse_scroll_zoom_tool');
		if (map.scrollWheelZoomEnabled()) {
			map.disableScrollWheelZoom();
			tool.style.color	= 'white';
		} else {
			map.enableScrollWheelZoom();
			tool.style.color	= 'green';
		}//end else
	}//end toggle_wheel_scrool()


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
			var icon = new GIcon();
			icon.image = icon_url_final;
			icon.iconSize = new GSize(icon_width, icon_height);
			icon.iconAnchor  = new GPoint(icon_width/2, 25);
			icon.infoWindowAnchor = new GPoint(icon_width/2, 14);
			var marker	= new GMarker(new GLatLng(latitude, longitude), icon);
		} else {
			var marker  = new GMarker(new GLatLng(latitude, longitude));
		}//end else

		if (street_view_enabled) {
			GEvent.addListener(marker, "click", function() {
				current_marker = marker;
				marker.openInfoWindowHtml(description);
				var latlng = marker.getLatLng();
				var div = document.getElementById("street_view");
				if (div && div.style.display !='block') {
					div.style.display = 'block';
				}//end if
				if (myPano) {
					myPano.setLocationAndPOV(latlng);
				}

			});
		} else {
			GEvent.addListener(marker, 'click',
			  function () {
			    //map.showMapBlowup(new GLatLng(latitude, longitude), 5, G_HYBRID_MAP);
			    current_marker = marker;
			    marker.openInfoWindowHtml(description);
			  }//end function
			)//end addListener
		}//end else

		map.addOverlay(marker);

		return marker;

	}//end addMarker()


	/**
	* This function is used to store added marker using a script
	*
	*/
	function storeMarker() {
		var lng  = document.getElementById("longitude").value;
		var lat  = document.getElementById("latitude").value;

		var getVars = ["?found=" , document.getElementById("found").value
		      ,  "&depth=" , document.getElementById("depth").value
		      ,  "&icon=" , document.getElementById("icon").value
		      ,  "&lng="  , lng
		      ,  "&lat="  , lat].join('');

		var request = GXmlHttp.create();

		//Open the request to storeMarker.php on the server
		request.open('GET', 'storeMarker.php' + getVars, true);
		request.onreadystatechange = function() {

		  if (request.readyState == 4) {
		      var xmlDoc  = request.responseXML;

		      // get the root node
		      var responseNode = xmlDoc.documentElement;

		      // get the type attribute
		      var type  = responseNode.getAttribute("type");

		      // get the content
		      var content = responseNode.firstChild.nodeValue;

		    if (type != 'success') {
		      alert(content);
		    } else {
		      //create a new marker
		      var latlng  = new GLatLng(parseFloat(lat), parseFloat(lng));
		      var iconImage  = responseNode.getAttribute("icon");
		      var marker  = createMarker(latlng, content, iconImage);
		      map.addOverlay(marker);
		      map.closeInfoWindow();
		    }//end else
		  }//end if
		}//end function
		request.send(null);
		return false;

	}//end storeMarker()


	function createMarker(latlng, html, iconImage) {
		if (iconImage!='') {
		  var icon = new GIcon();
		  icon.image = "http://delta.squiz.net/~hnguyen/Images/"+iconImage+".jpg";
		  icon.iconSize = new GSize(30, 30);
		  icon.iconAnchor  = new GPoint(14, 25);
		  icon.infoWindowAnchor = new GPoint(14, 14);
		  var marker  = new GMarker(latlng, icon);
		  //map.addOverlay(marker);

		} else {
		  var marker  = new GMarker(latlng);
		}//end else

		//var marker  = new GMarker(latlng);
		GEvent.addListener(marker, 'click', function() {
		  var markerHTML = html;
		  marker.openInfoWindowHtml(markerHTML);
		 }//end function
		)//end addListener
		return marker;
	}//end createMarker


	/**
	* This function is used to retrieve stored markers using a script
	*
	*/
	function retrieveMarkers() {
		var request  = GXmlHttp.create();

		request.open('GET', 'retrieveMarkers.php', true);
		request.onreadystatechange = function() {

		  if (request.readyState == 4) {
		    var xmlDoc = request.responseXML;

		    var markers = xmlDoc.documentElement.getElementsByTagName("marker");

		    for (var i = 0; i < markers.length; i++) {
		      var lng  = markers[i].getAttribute("lng");
		      var lat  = markers[i].getAttribute("lat");
		      if (lng && lat) {
		        var latlng  = new GLatLng(parseFloat(lat), parseFloat(lng));

		        var html  = ['<div><b>Found </b>',
		                markers[i].getAttribute("found"),
		                '</div><div><b>Depth</b> ',
		                markers[i].getAttribute("depth"),
		                '</div>'].join('');

		        var iconImage = markers[i].getAttribute("icon");
		        if (iconImage == null) iconImage = '';
		        var marker  = createMarker(latlng, html, iconImage);
		        map.addOverlay(marker);

		      }//end if

		    }//end for
		  }//end if

		}//end function
		request.send(null);

	}//end retrieveMarkers


	/**
	* This function is used to set the map type
	*
	*/
	function setMapType(mapType)
	{
		map.setMapType(mapType);
	}//end setMapType


	/**
	* This function is used to get the Earth Instance for Google Earth
	*
	*/
	function getEarthInstanceCB(object)
	{
		ge = object;
	}//end setMapType


	/**
	* This function is used to get the information about a marker
	*
	*/
	function getInfo(i)
	{
		GEvent.trigger(allMarkerList[i], "click");
	}//end getInfo()


	/**
	* This function is used to clear all the new markers from the map
	*
	*/
	function clearNewMarker(marker_array)
	{
		for (var i =0; i < marker_array.length; i++) {
			if (marker_array[i]) {
				marker_array[i].hide();
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
		var current_latlng	= marker.getLatLng();
		var current_lat		= current_latlng.lat();
		var current_lng		= current_latlng.lng();

		var closest_distance	= 0;
		var closest_marker		= null;

		for (var obj in allMarkerList) {
			if (!allMarkerList[obj].isHidden()) {
				var latlng	= allMarkerList[obj].getLatLng();
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
		var polyline = new GPolyline([
		  coordinate_1,
		  coordinate_2
		], color, 10);
		newPolylines.push(polyline);
		map.addOverlay(polyline);
	}//end drawLineBetweenMarkers()


	/**
	* Turn street view overlay on and off
	*/
	function toggleOverlay() {
	  if (!overlay) {
	    overlay = new GStreetviewOverlay();
	    map.addOverlay(overlay);
	  } else {
	    map.removeOverlay(overlay);
	    overlay = null;
	  }
	}//end toggleOverlay()


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
	function findLocationFromAddress(address, extra_text, icon_image, uid, street_view_enabled) {
		if (address) {
		  geocoder.getLatLng(
		    address,
		    function(point) {
		      if (!point) {
		        //alert(address + " not found");
		      } else {
		       if (!uid)  map.setCenter(point);

				if (icon_image != null && icon_image != '') {
					var icon_url_final = icon_image;
				} else {
					var icon_url_final = icon_url;
				}//end if
				if (icon_url_final != '' && icon_url_final != null) {
					var icon = new GIcon();
					icon.image = icon_url_final;
					icon.iconSize = new GSize(icon_width, icon_height);
					icon.iconAnchor  = new GPoint(icon_width/2, 25);
					icon.infoWindowAnchor = new GPoint(icon_width/2, 14);
					var marker  = new GMarker(point, icon);
				} else {
					var marker  = new GMarker(point);
				}//end else

		        map.addOverlay(marker);

				if (!extra_text) extra_text = '';
				GEvent.addListener(marker, 'click',
				  function () {
				    marker.openInfoWindowHtml(extra_text+' '+address);
				  }//end function
				)//end addListener

		        if (!uid) marker.openInfoWindowHtml(address);
		       	newMarkers.push(marker);
		       	updateAddressList(marker, address);
				current_marker = marker;

				if (street_view_enabled) {
					GEvent.addListener(marker,"click", function() {
						current_marker = marker;
						var latlng = marker.getLatLng();
						if (myPano) {
							myPano.setLocationAndPOV(latlng);
						}
						var div = document.getElementById("street_view");
						if (div && div.style.display !='block') {
							div.style.display = 'block';
						}//end if
					});
				} else {
					GEvent.addListener(marker, 'click',
					  function () {
					    //map.showMapBlowup(new GLatLng(latitude, longitude), 5, G_HYBRID_MAP);
					    current_marker = marker;
					    marker.openInfoWindowHtml(extra_text+' '+address);
					  }//end function
					)//end addListener
				}//end else

		      }
		    }
		 );
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

		var	latlng	= marker.getLatLng();
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
					marker_array[key_index][obj].show();
				}//end if
			}//end for
		} else {
			for (var obj in marker_array[key_index]) {
				if (obj != 'toggle') {
					marker_array[key_index][obj].hide();
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
			drawLineBetweenMarkers(last_marker.getLatLng(), closest_marker.getLatLng(), color);
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
