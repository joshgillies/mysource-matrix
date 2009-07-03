<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: js_api.inc,v 1.1 2009/07/03 04:43:49 bpearson Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Javascript API
*
* Purpose
*
*
* @author  Nic Hubbard <nic@zedsaid.com>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class JS_Api extends Page
{

    
	/**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
		$this->_ser_attrs = TRUE;
        parent::__construct($assetid);

    }//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name').'.js'), $link);

	}//end _createAdditional()
	
	
	/**
	* Print the frontend of the asset without the design
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		// Make sure the user can log in first
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		// Should we print our JS file, or send back some JSON?
		if (!isset($_REQUEST['key']) && !isset($_REQUEST['id']) && !isset($_REQUEST['type'])) {
			header('Content-Type: text/plain');
			echo $this->printJs();
			return;
		};

		// Overriding the matrix error handler so HTML doesn't get printed to the screen
		require_once dirname(__FILE__).'/js_api_error_handler.inc';
		$old_error_handler = set_error_handler('js_api_error_handler');
		
		// Get our JSON data that was sent, then encode it into an array
		$api_key = array_get_index($_REQUEST, 'key', '');;
		$id = array_get_index($_REQUEST, 'id', '');
		$type = array_get_index($_REQUEST, 'type', '');
		
		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		
		// Check our API Key
		$check_key = $this->attr('api_key');
		
		// Check to see if we even have an API Key
		if (!isset($check_key) || $check_key == '' || $api_key == '') {
			$this->returnError('You must enter an API key in order to use this asset');
			restore_error_handler();
			return;
		}
		// Check to see if the API key attribute matches the JSON key
		if ($check_key != $api_key) {
			$this->returnError('The API key does not match, please enter a valid key');
			restore_error_handler();
			return;
		}
		// Check to see if the asset id is real
		if ($id == '') {
			$this->returnError('You must enter an asset id');
			restore_error_handler();
			return;
		}
		// Check to see if the asset id is real
		try {
			$valid_assetid = $am->assetExists($id);
		} catch (Exception $e) {
			$this->returnError($id.' is not a valid asset id');
			restore_error_handler();
			return;
		}
		if (!$valid_assetid) {
			$this->returnError($id.' is not a valid asset id');
			restore_error_handler();
			return;
		}
		// Check to see if passed ID is under our root node, or, if we are using a root node
		if ($this->attr('root_node') != 0) {
			if (!$this->checkRoot($id)) {
				$this->returnError('You do not have permissions to access this asset');
				restore_error_handler();
				return;
			}
		}
		// Check to see if we have a function type
		if ($type == '') {
			$this->returnError('You must set a function type');
			restore_error_handler();
			return;
		}
		
		// Set some shortcuts
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
		$ra = $asset->readAccess();
		$wa = $asset->writeAccess();
		
		// Check to see what we should return
		switch ($type) {
			case	'getGeneral':
				if ($this->attr('get_general')) {
					if ($ra) {
						// Get general info about asset
						$data = array(
									'name'					=> 	$asset->name,
									'short_name'			=> 	$asset->short_name,
									'id'					=>	$asset->id,
									'type_code'				=>	$asset->type(),
									'data_path'				=> 	$asset->data_path,
									'status'				=>	$asset->getStatusDescription(),
									'created'				=>	$asset->created,
									'created_userid'		=>	$asset->created_userid,
									'updated'				=>	$asset->updated,
									'updated_userid'		=>	$asset->updated_userid,
									'published'				=>	$asset->published,
									'published_userid'		=>	$asset->published_userid,
									'status_changed'		=>	$asset->status_changed,
									'status_changed_userid'	=>	$asset->status_changed_userid,
								);
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getGeneral function must be activated');
				}//end else
				
			break;
				
			case	'getAttributes':
				if ($this->attr('get_attributes')) {
					if ($ra) {
						// Get available attributes
						$attributes = $am->getAssetTypeAttributes($asset->type());
						foreach ($attributes as $key => $val) {
							$data[$key] = $asset->attr($key);
						}// End foreach
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getAttributes function must be activated');
				}//end else
				
			break;
				
			case	'setAttribute':
				if ($this->attr('set_attribute')) {
					if ($wa) {
						if (isset($_REQUEST['attr_name']) && isset($_REQUEST['attr_val'])) {
							// Check to see if the passed name is even an attribute
							$attributes = $am->getAssetTypeAttributes($asset->type());
							if (array_key_exists($_REQUEST['attr_name'], $attributes)) {
								// Set our new values
								$this->setAttributeValue($id, $_REQUEST['attr_name'], $_REQUEST['attr_val']);
								$data['success'] = 'Attribute '.$_REQUEST['attr_name'].' has been set to '.$_REQUEST['attr_val'];
							}//end if
						} else {
							$this->returnError('Please enter both the attribute name and value');
						}//end else
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The setAttributes function must be activated');
				}//end else
				
			break;
				
			case	'getMetadata':
				if ($this->attr('get_metadata')) {
					if ($ra) {
						$data = $mm->getMetadataFieldValues($id);
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getMetadata function must be activated');
				}//end else
				
			break;
				
			case	'setMetadata':
				if ($this->attr('set_metadata')) {
					if ($wa) {
						if (isset($_REQUEST['field_id']) && isset($_REQUEST['field_val'])) {
							$this->setAssetMetadata($id, $_REQUEST['field_id'], $_REQUEST['field_val']);
							$data['success'] = 'Metadata field #'.$_REQUEST['field_id'].' has been set to: '.$_REQUEST['field_val'];
						} else {
							$this->returnError('Please enter both the field id and value');
						}//end else
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The setMetadata function must be activated');
				}//end else
				
			break;
				
			case	'trashAsset':
				if ($this->attr('trash_asset')) {
					if ($wa) {
						$this->trashAsset($id);
						$data['success'] = 'Asset #'.$id.' is now in the trash';
					} else {
						$this->returnError('You do not have permissions to delete this asset');
					}//end else
				} else {
					$this->returnError('The trashAsset function must be activated');
				}//end else
				
			break;
				
			default:	$this->returnError('This is not a valid function name');
			
		}// End switch
 
		// Send our data as JSON
		if (isset($data)) {
			if (function_exists('json_encode') === FALSE) {
				require_once 'Services/JSON.php';
				$json = new Services_JSON();
				$output = $json->encode($data);
			} else {
				$output = json_encode($data);
			}// End else

			// Send our JSON
			echo $output;
			
		}// End if

		// Restore error handler
		restore_error_handler();

	}//end printFrontend()
	
	
	/**
	* Description: This operation will send an asset to the trash
	*
    * @param string  $id  The id of the asset we want to delete
    *
	*
	* @return void
	* @access public
	*/
	function trashAsset($id)
	{
		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		
		if (!empty($id)) {
			$already_in_trash = $am->assetInTrash($id, TRUE);

			$am->acquireLock($id, 'all');
			if ($already_in_trash) {
				$this->returnError('Asset #'.$id.' is already in the trash');
				return;
			} else {
				$result	= $am->trashAsset($id);
			}//end else
			$am->releaseLock($id, 'all');

		} else {
			$this->returnError('Asset #'.$id.' is not valid. Please provide a valid Asset ID');
			return;
		}//end else

	}//end TrashAsset()
	
	
	/**
	* Description: This operation will return set the attribute value of an asset based on assetid and attribute name
	*
	* @param string 	$assetid	The ID of the asset in query
	* @param string 	$attr_name	The name of the attribute
	* @param string 	$attr_val	The new value of the attribute
    *
	* @return void
	* @access public
	*/
	function setAttributeValue($assetid, $attr_name, $attr_val)
	{
		if (!empty($assetid) && !empty($attr_name) && !empty($attr_val)) {
			// Shortcuts
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$am = $GLOBALS['SQ_SYSTEM']->am;
			
			// Change our values
			$am->acquireLock($assetid, 'attributes');
			$asset->setAttrValue($attr_name, $attr_val);
			$asset->saveAttributes();
			$am->releaseLock($assetid, 'attributes');
			$am->forgetAsset($asset);
		} else {
			$this->returnError('Please make sure to provide AssetID, AttributeName and AttributeValue');
		}// End else

	}// End setAttributeValue()

	
	/**
	* Description: This operation will set the value for a metadata field of an asset
	*
	* @param string 	$assetid	The asset to regenerate metadata for
	* @param string 	$fieldid	The metadata field id
	* @param string 	$new_value	The new value for the field
	*
	* @return void
	* @access public
	*/
	function setAssetMetadata($assetid, $fieldid, $new_value)
	{
		if (!empty($assetid) && !empty($fieldid)) {
			// Shortucts
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$am = $GLOBALS['SQ_SYSTEM']->am;
			
			$field	= $am->getAsset($fieldid);
			if (!($field instanceof Metadata_Field)) {
				$this->returnError('The Field ID provided does not belong to a metadata field');
			}//end if
			$am->acquireLock($assetid, 'metadata');

			$field_asset	= $am->getAsset($fieldid);
			$field_name		= $field_asset->attr('name');


			$metadata_info	= Array (
								$fieldid	=> Array (
												Array (
													'name'	=> $field_name,
													'value'	=> $new_value,
												),
											   ),
							  );
			// Set metadata and generate the content file
			$s_result = $mm->setMetadata($assetid, $metadata_info);
			$g_result = $mm->generateContentFile($assetid);
			$am->releaseLock($assetid, 'metadata');
			if ($s_result && $g_result) {
				return Array (
						'SetAssetMetadataResult'	=> $result,
					   );
			} else {
				$this->returnError('Unable To Regenerate Metadata For Asset');
			}// End else
		} else {
			$this->returnError('Please provide both AssetID and FieldID of the metadata field');
		}// End else

	}//end setAssetMetadata()
	
	
	/**
	* Description: Check out see if the passed asset is under the root restriction
	*
	* @param string	 $id	The requested id
	*
	* @return TRUE
	* @access public
	*/
	function checkRoot($id)
	{
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($id);
		return isset($parents[$this->attr('root_node')]);
	
	}//end checkRoot
	
	
	/**
    * Returns an error in JSON format
	*
	* @param string	 $error	The error to send back as JSON
	*
    * @access public
    */
    function returnError($error)
	{
		// Set our error
		trigger_error($error, E_USER_WARNING);
	
		// Send error as JSON so they are not confused why the function quit
		$data['error'] = $error;
		
		// Send our data as JSON
		if (isset($data)) {
			if (function_exists('json_encode') === FALSE) {
				require_once 'Services/JSON.php';
				$json = new Services_JSON();
				$output = $json->encode($data);
			} else {
				$output = json_encode($data);
			}// End else
			
			// Send our JSON
			echo $output;
			
		}// End if
		
	}// End returnError()
	
	
	/**
	* Description: Function that prints our JS file if needed
	*
	* @access public
	*/
	function printJs()
	{
	?>
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

/**
* This will create an ajax request
*
* @version $Revision: 0.1
*/
function createRequest() 
{
    var request;
    try {
        request = new XMLHttpRequest();
    } catch (trymicrosoft) {
        try {
            request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (othermicrosoft) {
            try {
                request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (failed) {
                request = false;
            }// End catch
        }// End catch
    }// End catch
    
    if (!request) {
        alert('Your browser does not support Ajax');
    }
    
    return request;
    
}// End createRequest


/**
* Turns JSON into a javascript object
*
* @param string		json			The JSON string to convert
*
* @version $Revision: 0.1
*/
function jsonToObj(json)
{
    // Make the conversion
    // Don't worry, even the creator of JSON says eval is ok here
    var jsonObj = eval('(' + json + ')');
    
    return jsonObj;

}// End jsonToObj
    

/**
* Our default callback
*
* @param string		ajaxRequest		The ajax function
* @param string		dataCallback	Callback that happens after success
*
* @version $Revision: 0.1
*/
function success(ajaxRequest, dataCallback) 
{
    if (ajaxRequest.readyState == 4) {
        if (ajaxRequest.status == 200) {
            if (ajaxRequest.responseText !== '' || ajaxRequest.responseText !== 'undefined' || ajaxRequest.responseText !== null) {
                var response = jsonToObj(ajaxRequest.responseText);
                // Custom callback
                dataCallback(response);
            }// End if
            
        }// End if
        
    }// End if
     
}// End success


/**
* This will return our api key
*
* @param string		api_key		The api key of our Javascript API Asset
*
* @version $Revision: 0.1
*/
function setApiKey(api_key) 
{
    // Make this into a global variable
    window.api_key = api_key;
	
}// End setApiKey
  

/**
* This will return general information about the asset
*
* @param string		asset_id		Id of the asset we are getting info for
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function getGeneral(asset_id, dataCallback) 
{
    // Create blank function
    var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
    
    // Build our string
    var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getGeneral&id=' + asset_id;
    
    // Create an instance of our ajax object
    var ajaxRequest = createRequest();
    // Open request
    ajaxRequest.open('POST', url, true);
    // Custom callback
    ajaxRequest.onreadystatechange = function() {
        success(ajaxRequest, dataCallback);
    }
    ajaxRequest.send(null);
    
}// End getGeneral
    

/**
* This will return attributes of the specific asset
*
* @param string		asset_id		Id of the asset we are getting info for
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function getAttributes(asset_id, dataCallback) 
{
    // Create blank function
    var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
    
    // Build our string
    var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getAttributes&id=' + asset_id;
    
    // Create an instance of our ajax object
    var ajaxRequest = createRequest();
    // Open request
    ajaxRequest.open('POST', url, true);
    // Custom callback
    ajaxRequest.onreadystatechange = function() {
        success(ajaxRequest, dataCallback);
    }
    ajaxRequest.send(null);
    
}// End getAttributes


/**
* This will set an attribute value
*
* @param string		asset_id		Id of the asset we are getting info for
* @param string		attr_name		Name of the attribute to change
* @param string		attr_val		Value to change the attribute to
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function setAttribute(asset_id, attr_name, attr_val, dataCallback) 
{
    // Create blank function
    var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
    
    // Build our string
    var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setAttribute&id=' + asset_id + '&attr_name=' + attr_name + '&attr_val=' + attr_val;
    
    // Create an instance of our ajax object
    var ajaxRequest = createRequest();
    // Open request
    ajaxRequest.open('POST', url, true);
    ajaxRequest.onreadystatechange = function() {
        success(ajaxRequest, dataCallback);
    }
    ajaxRequest.send(null);
    
}// End setAttribute


/**
* This will return a metadata value for the passed metadata name
*
* @param string		asset_id		Id of the asset we are getting info for
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function getMetadata(asset_id, dataCallback) 
{
    // Create blank function
    var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
    
    // Build our string
    var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getMetadata&id=' + asset_id;
    
    // Create an instance of our ajax object
    var ajaxRequest = createRequest();
    // Open request
    ajaxRequest.open('POST', url, true);
    ajaxRequest.onreadystatechange = function() {
        success(ajaxRequest, dataCallback);
    }
    ajaxRequest.send(null);
    
}// End getMetadata


/**
* This will set a metadata value
*
* @param string		asset_id		Id of the asset we are getting info for
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function setMetadata(asset_id, field_id, field_val, dataCallback) 
{
    // Create blank function
    var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
    
    // Build our string
    var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setMetadata&id=' + asset_id + '&field_id=' + field_id + '&field_val=' + field_val;
    
    // Create an instance of our ajax object
    var ajaxRequest = createRequest();
    // Open request
    ajaxRequest.open('POST', url, true);
    ajaxRequest.onreadystatechange = function() {
        success(ajaxRequest, dataCallback);
    }
    ajaxRequest.send(null);
    
}// End setMetadata


/**
* This will send an asset to the trash
*
* @param string		asset_id		Id of the asset to delete
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function trashAsset(asset_id, dataCallback)
{
    // Create blank function
    var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
    
    // Build our string
    var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=trashAsset&id=' + asset_id;
    
    // Create an instance of our ajax object
    var ajaxRequest = createRequest();
    // Open request
    ajaxRequest.open('POST', url, true);
    ajaxRequest.onreadystatechange = function() {
        success(ajaxRequest, dataCallback);
    }
    ajaxRequest.send(null);
        
}// End trashAsset

	<?php
	}//end printJs


}//end class
?>
