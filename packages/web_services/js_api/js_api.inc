<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: js_api.inc,v 1.69.2.2 2011/03/10 00:18:55 akarelia Exp $
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
* @version $Revision: 1.69.2.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class JS_Api extends Page
{

	
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
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

		// Create a random key
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes');
		$key = rand(1000000000, 9999999999);
		$this->setAttrValue('api_key', $key);
		$this->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
		
		return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name').'.js'), $link);

	}//end _createAdditional()


    /**
    * Returns an array of all the permitted links type, the type asset and the cardinality
    *
    * @return array
    * @access private
    * @see Asset::_getAllowLinks()
    */
    public function _getAllowedLinks()
    {
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_NOTICE] = Array('asset' => Array('card' => 'M', 'exclusive' => FALSE));
		return $links;

    }//end _getAllowedLinks()


	/**
	* Print the frontend of the asset without the design
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$replace_these = Array ('%26' , '%23' , '%3F' , '%2B');
		$replace_with = Array ('&' , '#' , '?' , '+');
		// Make sure the user can log in first
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		// Should we print our JS file, or send back some JSON?
		if (!isset($_REQUEST['key']) && !isset($_REQUEST['id']) && !isset($_REQUEST['type'])) {
			header('Content-Type: text/javascript');
			echo $this->printJs();
			return;
		}

		// Overriding the matrix error handler so HTML doesn't get printed to the screen
		require_once dirname(__FILE__).'/js_api_error_handler.inc';
		$old_error_handler = set_error_handler('js_api_error_handler');

		// Get our JSON data that was sent, then encode it into an array
		$api_key = array_get_index($_REQUEST, 'key', '');
		$id = array_get_index($_REQUEST, 'id', '');
		$type = array_get_index($_REQUEST, 'type', '');

		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$ignore = $this->attr('ignore_permissions');

		// Set our API Key
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

		// Check to see if we have a function type
		if (empty($type)) {
			$this->returnError('You must set a function type');
			restore_error_handler();
			return;
		}

		$temp_server_vars = $_SERVER['PHP_SELF'];

		// #4372  Feature Request : JS API ability to specify _admin/_edit access
		// Lets populate our $_REQUEST var so that we can use it later :)
		$_REQUEST['force_simple_edit'] = $this->attr('force_simple_edit');

		$temp_server_vars = $_SERVER['PHP_SELF'];

		if (isset($_SERVER['HTTP_REFERER']) && !empty($_SERVER['HTTP_REFERER'])) {
			$referer_url = str_replace(current_protocol().'://', '', $_SERVER['HTTP_REFERER']);

			// if we are in Backend or limbo remove everything after _edit or _admin
			// or else getAssetFromURL will get confused and wont return us correct asset
			if (strpos($_SERVER['HTTP_REFERER'], SQ_CONF_LIMBO_SUFFIX) !== FALSE) {
				$referer_url = preg_replace('/'.SQ_CONF_LIMBO_SUFFIX.'(.)*/', '', $referer_url);
			} else if (strpos($referer_url, SQ_CONF_BACKEND_SUFFIX) !== FALSE) {
				$referer_url = preg_replace('/'.SQ_CONF_BACKEND_SUFFIX.'(.)*/', '', $referer_url);
			}

			$url_asset_object = $am->getAssetFromURL(current_protocol(), $referer_url, TRUE, TRUE);
			if (!is_null($url_asset_object) ) {
				$_SERVER['PHP_SELF'] = str_replace(current_protocol().'://'.$_SERVER['HTTP_HOST'], '', $_SERVER['HTTP_REFERER']);
			}

			if (strpos($_SERVER['HTTP_REFERER'], SQ_CONF_LIMBO_SUFFIX) !== FALSE || $this->attr('force_simple_edit')) {
				$_SERVER['PHP_SELF'] = preg_replace('/'.SQ_CONF_LIMBO_SUFFIX.'(.)*/', '', $_SERVER['PHP_SELF']);
				$_SERVER['PHP_SELF'] = preg_replace('/'.SQ_CONF_BACKEND_SUFFIX.'(.)*/', '', $_SERVER['PHP_SELF']);
				$_REQUEST['force_simple_edit'] = TRUE;
			}
		}

		// Non asset based functions need to be handled separately
		switch ($type) {
			case 'getAssetTypes':
				if ($this->attr('get_asset_types')) {
						$data = $am->getAssetTypes(TRUE, TRUE);
						// Sort in A-Z order
						ksort($data);
				} else {
					$this->returnError('The getAssetTypes function must be activated');
					restore_error_handler();
					return;
				}//end else

			break;			

			case	'trashAsset':
				if ($this->attr('trash_asset')) {
					if (isset($_REQUEST['assetid']) && !empty($_REQUEST['assetid'])) {
						$assetids = explode('\\,', $_REQUEST['assetid']);
						$data = $this->trashAsset($assetids);
					} else {
						$this->returnError('Please Enter Asset Id(s) to move to Trash');
					}
				} else {
					$this->returnError('The trashAsset function must be activated');
				}//end else

			break;

			case	'getChildCount':

				if ($this->attr('get_general')) {
					if (isset($_REQUEST['asset_id']) && !empty($_REQUEST['asset_id'])) {
						$assetid = $_REQUEST['asset_id'];
						// Check to see how deep we should go
						if ($_REQUEST['depth'] == '0') {
							$level = NULL;
						} else {
							$level = $_REQUEST['depth'];
						}
						$children = $this->getChildren($assetid, $level);
						$data['child_count'] = count($children);
					} else {
						$this->returnError('Please Enter Asset Id to get the child count');
					}
				} else {
					$this->returnError('The getGeneral function must be activated');
				}//end else

			break;

			case	'updateMultipleLinks':

				if ($this->attr('update_link')) {
						$links = Array();
						$links['parents']  = explode('\\,', $_REQUEST['parent_id']);
						$links['children'] = explode('\\,', $_REQUEST['child_id']);
						$links['existing_link_type'] = explode('\\,', $_REQUEST['existing_link_type']);
						$links['existing_link_value'] = explode('\\,', $_REQUEST['existing_link_value']);
						$links['link_type'] = explode('\\,', $_REQUEST['link_type']);
						$links['link_value'] = explode('\\,', $_REQUEST['link_value']);
						$links['sort_order'] = explode('\\,', $_REQUEST['sort_order']);
						$links['lock'] = explode('\\,', $_REQUEST['locked']);

						foreach ($links as $elements) {
							if (count($elements) != count($links['parents'])) {
								$this->returnError('Please provide correct number of link elements');
								return FALSE;
							}
						}

						if(!$data) {
							//looks like correct number of elements are passed
							for ($i = 0;  $i < count($links['parents']); $i++) {
								// Update our link
								if($links['children'][$i] == 'Undefined' || $links['children'][$i] == '' ) {
									$data[$i]['error'] = 'Child Id whose link is to be update was not provided';
									continue;
								} else if ($links['parents'][$i] == 'Undefined' || $links['parents'][$i] == ''){
									$data[$i]['error'] = 'Parent Id whose link is to be update was not provided';
									continue;
								}

								$child_asset = $am->getAsset($links['children'][$i]);
								if(is_null($child_asset)) {
									$data[$i]['error'] = 'Asset #'.$links['children'][$i].' does not exist';
									continue;
								}

								$parent_asset = $am->getAsset($links['parents'][$i]);
								if(is_null($parent_asset)) {
									$data[$i]['error'] = 'Asset #'.$links['parents'][$i].' does not exist';
									continue;
								}

								if ($child_asset->writeAccess()) {

									$ext_link_type = defined($links['existing_link_type'][$i]) ? constant($links['existing_link_type'][$i]) : SQ_LINK_TYPE_1;
									$ext_link_value = str_replace($replace_these, $replace_with,  $links['existing_link_value'][$i]);
									$link_type = defined($links['link_type'][$i]) ? constant($links['link_type'][$i]) : SQ_LINK_TYPE_1;
									$link_value = str_replace($replace_these, $replace_with, $$links['link_value'][$i]);
									$data[$i] = $this->updateLink(
																	$links['parents'][$i] ,
																	$links['children'][$i] ,
																	$ext_link_type,
																	$ext_link_value,
																	$link_type,
																	$link_value ,
																	($links['sort_order'][$i] != '' && $links['sort_order'][$i] != 'undefined') ? $links['sort_order'][$i] : NULL ,
																	($links['lock'][$i] != '' && $links['lock'][$i] != 'undefined') ? $links['lock'][$i] : TRUE
																);
								} else {
									$data[$i] = 'Not enough permission to update link between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and for Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.')';
								}
							}
						}
						if (!$data) return;
				} else {
					$this->returnError('The updateLink function must be activated');
				}//end else

			break;

			case	'removeMultipleLinks':

				$data = Array();

				if ($this->attr('remove_link')) {
					$links = Array();
					$links['parents']  = explode('\\,', $_REQUEST['parent_id']);
					$links['children'] = explode('\\,', $_REQUEST['child_id']);
					$links['link_type'] = explode('\\,', $_REQUEST['link_type']);
					$links['link_value'] = explode('\\,', $_REQUEST['link_value']);

					foreach ($links as $elements) {
						if (count($elements) != count($links['parents'])) $this->returnError('Please provide correct number of link elements');
					}

					//looks like correct number of elements are passed
					for ($i = 0;  $i < count($links['parents']); $i++) {
						// Update our link
						if($links['children'][$i] == 'Undefined' || $links['children'][$i] == '' || $links['children'][$i] == NULL) {
							$data[$i]['error'] = 'Child Id whose link is to be update was not provided';
							continue;
						} else if ($links['parents'][$i] == 'Undefined' || $links['parents'][$i] == ''|| $links['children'][$i] == NULL){
							$data[$i]['error'] = 'Parent Id whose link is to be update was not provided';
							continue;
						}

						$child_asset = $am->getAsset($links['children'][$i]);
						if(is_null($child_asset)) {
							$data[$i]['error'] = 'Asset #'.$links['children'][$i].' does not exist';
							continue;
						}

						$parent_asset = $am->getAsset($links['parents'][$i]);
						if(is_null($parent_asset)) {
							$data[$i]['error'] = 'Asset #'.$links['parents'][$i].' does not exist';
							continue;
						}

						if ($child_asset->writeAccess()) {

							$link_value = str_replace($replace_these, $replace_with, $links['link_value'][$i]);
							$link_type = defined($links['link_type'][$i]) ? constant($links['link_type'][$i]) : SQ_LINK_TYPE_1;

							$data[$i] = $this->removeLink($parent_asset->id, $child_asset->id, $link_type, $link_value);
						} else {
							$data[$i] = 'Not enough permission to remove link between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and for Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.')';
						}
					}

				} else {
					$this->returnError('The removeLink function must be activated');
				}//end else

			break;


			case	'importAssetsFromXML':

				$data = Array();
				if ($this->attr('import_assets')) {
					$filePath = $_REQUEST['filePath'];
					$import_under = $_REQUEST['id'];

					//check if the file exists
					if (file_exists($filePath) || $filePath == 'no_file_provided') {
						//check 2 make sure it is a XML file
						if (strtolower(substr($filePath, -3)) == 'xml') {
							//looks all good...makes sure the asset id provided is valid
							if($am->assetExists($import_under)) {
								// set HIPO running_vars
								$vars['file_info'] = $filePath;
								$vars['create_under_assetid'] = $import_under;
								$vars['delete_after_import'] = FALSE;

								// run HIPO job
								$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
								$errors = $hh->freestyleHipo('hipo_job_import_assets_from_xml', $vars, SQ_PACKAGES_PATH.'/import_tools/hipo_jobs');
								if (empty($errors)) {
									$data['success'] = 'Assets sucessfully imported under Asset #'.$import_under;
								} else {
									$data['error'] = 'Error occured while importing assets  under Asset #'.$import_under;
								}

							} else {
								$this->returnError('Assetid provided to import assets under is not valid');
							}
						} else {
							$this->returnError('File provided is not XML');
						}
					} else {
						$this->returnError('No File path provided OR File does not exist for the provided path');
					}					
				} else {
					$this->returnError('The importAssetsFromXML function must be activated');
				}//end else

			break;


			case	'getRoles':
				if ($this->attr('get_roles')) {
					$assetid = isset($_REQUEST['assetid']) && $_REQUEST['assetid'] != '' ? $_REQUEST['assetid'] : NULL;
					$roleid = isset($_REQUEST['roleid']) && $_REQUEST['roleid'] != '' ? $_REQUEST['roleid'] : NULL;
					$userid = isset($_REQUEST['userid']) && $_REQUEST['userid'] != '' ? $_REQUEST['userid'] : NULL;
					$include_assetid = isset($_REQUEST['include_assetid']) && $_REQUEST['include_assetid'] != '' ? $_REQUEST['include_assetid'] : FALSE;
					$include_globals = isset($_REQUEST['include_globals']) && $_REQUEST['include_globals'] != '' ? $_REQUEST['include_globals'] : FALSE;
					$expand_groups = isset($_REQUEST['expand_groups']) && $_REQUEST['expand_groups'] != '' ? $_REQUEST['expand_groups'] : FALSE;
					$inc_dependants = isset($_REQUEST['inc_dependants']) && $_REQUEST['inc_dependants'] != '' ? $_REQUEST['inc_dependants'] : TRUE;
					$data = $am->getRole($assetid, $roleid, $userid, $include_assetid, $include_globals, $expand_groups, $inc_dependants);
					if(empty($data))
						$data[] = 'No roles data is found';
				} else {
					$this->returnError('The getRoles function must be activated');
				}//end else

			break;
			
			default:
				// Pass through so the asset based commands can work
		}//end if


		// Send our data as JSON
		if (isset($data)) {
			$this->returnJSON($data, $type);
			// Restore error handler
			restore_error_handler();
			return;
		}//end if

		// Check to see if we are using an id or a URL
		$asset = $this->setAsset($id);
		if (!$asset) return;

		// Check to see if passed ID is under our root node, or, if we are using a root node		
		if (!$this->checkRoot($asset)) {
			$this->returnError('You do not have permissions to access this asset');
			restore_error_handler();
			return;
		}

		// Set some shortcuts
		$ra = $asset->readAccess();

		//check if there is any runing workflow schema on it
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_wfs = $wfm->getSchemas($asset->id, TRUE, TRUE);
		if(!empty($running_wfs)) {
			$wa = $asset->writeAccess();
		} else {
			$wa = $asset->writeAccess('', Array(), FALSE);
		}

		// Check to see what we should return
		switch ($type) {
			case	'getGeneral':
				if ($this->attr('get_general')) {
					if ($ra) {
						$get_attributes = isset($_REQUEST['get_attributes']) ? $_REQUEST['get_attributes'] : FALSE;
						$data = $this->getGeneralInfo($asset, $get_attributes);
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
							if ($key == 'api_key') {
								// Added security to make sure no one can get API Keys from other API Assets
								$data[$key] = '';
							} else {
								$data[$key] = $asset->attr($key);
							}
						}//end foreach
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getAttributes function must be activated');
				}//end else

			break;

			case	'setMultipleAttributes' :
			case	'setAttribute':
				if ($this->attr('set_attribute')) {
					if ($wa) {
						if (isset($_REQUEST['attr_name']) && isset($_REQUEST['attr_val'])) {
							$attr_names = explode('\\,', $_REQUEST['attr_name']);
							$attr_vals  = explode('\\,', $_REQUEST['attr_val']);

							foreach($attr_vals as $index =>  $attr_val) {
								$attr_vals[$index] = str_replace($replace_these, $replace_with, $attr_val);
							}

							// Set our new values
							$success = $this->setAttributeValue($asset->id, $attr_names, $attr_vals);;
							if($success) {
								$data = $success; 
							} else {
								$data = 'Attribute for Asset #'.$asset->id.' were not set. Check Error Log for more information';
							}
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
						$data = $mm->getMetadataFieldValues($asset->id);
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getMetadata function must be activated');
				}//end else

			break;

			case 	'setMetadataAllFields' :
			case	'setMetadata':
				if ($this->attr('set_metadata')) {
					if ($wa) {
						if (isset($_REQUEST['field_id']) && isset($_REQUEST['field_val'])) {
							require_once SQ_CORE_PACKAGE_PATH.'/metadata/metadata_field/metadata_field.inc';

							$field_ids = explode('\\,', $_REQUEST['field_id']);
							$field_vals = explode('\\,', $_REQUEST['field_val']);

							foreach($field_vals as $index =>  $fieldVal) {
								$field_vals[$index] = str_replace($replace_these, $replace_with, $fieldVal);
								$field_vals[$index] = Metadata_Field::encodeValueString($field_vals[$index], Array());
							}

							$result = $this->setAssetMetadata($asset->id, $field_ids, $field_vals);
							if($result) {
								$i = 0;
								foreach($field_vals as $index =>  $fieldVal) {
									$value = str_replace($replace_these, $replace_with, $fieldVal);
									$to_decode = Array('\=', '\;', '\\\\');
									$decoded_str = Array('=', ';', '\\');
									$value = str_replace($to_decode, $decoded_str, $value);
									$data['success'][] = 'Metadata field #'.$field_ids[$i].' has been successfully set to "'.$value.'" for Asset "'.$asset->name.'" (#'.$asset->id.')';
									$i++;
								}
							}
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

			case	'getChildren':
				if ($this->attr('get_children')) {
					if ($ra) {
						if (empty($_REQUEST['depth']) && $_REQUEST['depth'] != '0') {
							$this->returnError('Please set a depth limit');
							return;
						}
						// Check to see how deep we should go
						if ($_REQUEST['depth'] == '0') {
							$level = NULL;
						} else {
							$level = $_REQUEST['depth'];
						}

						// lets check for the type_codes, link_types, and link_values
						$type_codes = (isset($_REQUEST['type_code']) && $_REQUEST['type_code'] != '') ? explode('\\,', $_REQUEST['type_code']) : Array();
						$link_types = (isset($_REQUEST['link_type']) && $_REQUEST['link_type'] != '') ? explode('\\,', $_REQUEST['link_type']) : Array();
						$link_values = (isset($_REQUEST['link_value']) && $_REQUEST['link_value'] != '') ? explode('\\,', $_REQUEST['link_value']) : Array() ;
						$get_attributes = isset($_REQUEST['get_attributes']) ? $_REQUEST['get_attributes'] : FALSE;

						$data = $this->getChildren($id, $level, $type_codes, $link_types, $link_values, $get_attributes);
						if (empty($data)) {
							$this->returnError('Asset #'.$asset->id.' has no children');
							return;
						}
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getChildren function must be activated');
				}//end else

			break;

			case	'getParents':
				if ($this->attr('get_parents')) {
					if ($ra) {
						if ($asset->id == 1) {
							$this->returnError('The root asset does not have any parents');
							return;
						}
						if (empty($_REQUEST['depth']) && $_REQUEST['depth'] != '0') {
							$this->returnError('Please set a depth limit');
							return;
						}
						// Check to see how deep we should go
						if ($_REQUEST['depth'] == '0') {
							$level = NULL;
						} else {
							$level = $_REQUEST['depth'];
						}

						// lets check for the type_codes, link_types, and link_values
						$type_codes = (isset($_REQUEST['type_code']) && $_REQUEST['type_code'] != '') ? explode('\\,', $_REQUEST['type_code']) : Array();
						$link_types = (isset($_REQUEST['link_type']) && $_REQUEST['link_type'] != '') ? explode('\\,', $_REQUEST['link_type']) : Array();
						$link_values = (isset($_REQUEST['link_value']) && $_REQUEST['link_value'] != '') ? explode('\\,', $_REQUEST['link_value']) : Array() ;
						$get_attributes = isset($_REQUEST['get_attributes']) ? $_REQUEST['get_attributes'] : FALSE;

						$data = $this->getParents($asset->id, $level, $type_codes, $link_types, $link_values, $get_attributes);
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getParents function must be activated');
				}//end else

			break;

			case	'getPermissions':
				if ($this->attr('get_permissions')) {
					if ($ra) {
						$level = $_REQUEST['level'];
						if (empty($level)) {
							$this->returnError('Please set a permissions level');
							return;
						}
						if (!is_numeric($level)) {
							$this->returnError('Permissions level must be numeric');
							return;
						}
						if ($level > 3) {
							$this->returnError($level.' is not a valid permissions level');
							return;
						}
						
						$data = $this->getPermission($asset->id, $level);
						if (!$data) return;
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getPermissions function must be activated');
				}//end else

			break;

			case	'createAsset':
				if ($this->attr('create_asset')) {
					if ($wa || $ignore) {
						// These must be set, so we don't need to check these
						if (isset($_REQUEST['type_code']) && isset($_REQUEST['asset_name'])) {
							isset($_REQUEST['type_code']) ? $type_code = $_REQUEST['type_code'] : $type_code = '';
							isset($_REQUEST['asset_name']) ? $asset_name = str_replace($replace_these, $replace_with, $_REQUEST['asset_name']) : $asset_name = '';
							isset($_REQUEST['link_type']) ? $link_type = $_REQUEST['link_type'] : $link_type = 1;
							isset($_REQUEST['link_value']) ? $link_value = str_replace($replace_these, $replace_with, $_REQUEST['link_value']) : $link_value = '';
							isset($_REQUEST['sort_order']) ? $sort_order = $_REQUEST['sort_order'] : $sort_order = '';
							isset($_REQUEST['is_dependant']) ? $is_dependant = $_REQUEST['is_dependant'] : $is_dependant = 0;
							isset($_REQUEST['is_exclusive']) ? $is_exclusive = $_REQUEST['is_exclusive'] : $is_exclusive = 0;
							if (isset($_REQUEST['extra_attributes']) && $_REQUEST['extra_attributes'] == '1') $extra_attributes = TRUE;
							
							// Create our asset
							$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
							$data = $this->createAsset($asset->id, $type_code, $asset_name, $link_type, $link_value, $sort_order, $is_dependant, $is_exclusive, $extra_attributes);
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
							if (empty($data)) {
								if (!$data) {
									$this->returnError('You are not allowed to create this asset type');
									return;
								}//end if
								unset($data);
								$this->returnError('Unable to create asset');
							}//end if
						} else {
							$this->returnError('You must set the parent id, type code and asset name to create an asset');
						}
					} else {
						$this->returnError('You do not have permissions to create an asset');
					}//end else
				} else {
					$this->returnError('The createAsset function must be activated');
				}//end else

			break;

			case	'getLocksInfo':
				if ($this->attr('get_locks_info')) {
					if ($ra) {
						$screen = $_REQUEST['screen'];
						$response = $this->getLocksInfo($asset->id, $screen);
						if (empty($response)) {
							$data[] = 'No lock are held by anyone on Asset "'.$asset->name.'" (#'.$asset->id.')';
						} else {
							foreach ($response as $screen_lock => $lock_info) {
								if ($screen_lock == '0') $screen_lock = $screen;
								$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock_info['userid']);
								$expires_in = easy_time_total(($lock_info['expires'] - time()), TRUE);
								if (!$expires_in) $expires_in = '1 second';
								$data[] = '"'.$screen_lock.'" locks are held by User "'.$user->name.'" (#'.$lock_info['userid'].') for Asset "'.$asset->name.'" (#'.$asset->id.'). This is due to expire in '.$expires_in;
							}
						}// end if
					} else {
						$this->returnError('You do not have permission to access this asset');
					}//end else
				} else {
					$this->returnError('The getLocksInfo function must be activated');
				}
				
			break;

			case	'acquireLock':
				if ($this->attr('acquire_lock')) {
					if ($wa) {
						$screen = $_REQUEST['screen'];
						$dependants_only = isset($_REQUEST['dependants_only']) ? $_REQUEST['dependants_only'] : TRUE ;
						$force_acquire = isset($_REQUEST['force_acquire']) ? $_REQUEST['force_acquire'] : FALSE;
						$response = $this->acquireLocks($asset, $screen, $dependants_only, $force_acquire);
						if (empty($response)) {
							$data[] = '"'.$screen.'" locks are now acquired for Asset "'.$asset->name.'" (#'.$asset->id.')';
						} else {
							$data = $response;
						}// end if
					} else {
						$this->returnError('You do not have permission to access this asset');
					}//end else
				} else {
					$this->returnError('The acquireLock function must be activated');
				}//end else

			break;

			case	'releaseLock':
				if ($this->attr('release_lock')) {
					if ($wa) {
						$screen = $_REQUEST['screen'];
						$response = $this->getLocksInfo($asset->id, $screen);
						if (empty($response)) {
							$data[] =  'No lock are held by anyone on Asset "'.$asset->name.'" (#'.$asset->id.')';
						} else {
							$response = $am->releaseLock($asset->id, $screen);
								if ($response) {
									$data[] ='"'.$screen.'" locks are now released for Asset "'.$asset->name.'" (#'.$asset->id.')';
								} else {
								$this->returnError('You cannot release locks on this asset');
							}// end if
						}//end else
					} else {
						$this->returnError('You do not have permission to access this asset');
					}//end else
				} else {
					$this->returnError('The releaseLock function must be activated');
				}//end else

			break;

			case	'createLink':
				if ($this->attr('create_link')) {
					if ($wa) {
						if (isset($_REQUEST['parent_id'])) {
							// Check to see if we are using an id or a URL
							$parent = $this->setAsset($_REQUEST['parent_id']);
							if (!$parent) return;
						} else {
							$this->returnError('A parent asset is required');
							return;
						}
						isset($_REQUEST['link_type']) ? $link_type = $_REQUEST['link_type'] : $link_type = 1;
						isset($_REQUEST['link_value']) ? $link_value = $_REQUEST['link_value'] : $link_value = '';
						isset($_REQUEST['sort_order']) ? $sort_order = $_REQUEST['sort_order'] : $sort_order = '';
						isset($_REQUEST['is_dependant']) ? $is_dependant = $_REQUEST['is_dependant'] : $is_dependant = 0;
						isset($_REQUEST['is_exclusive']) ? $is_exclusive = $_REQUEST['is_exclusive'] : $is_exclusive = 0;
						$data = $this->createLink($parent->id, $asset->id, $link_type, $link_value, $sort_order, $is_dependant, $is_exclusive);
					} else {
						$this->returnError('You do not have permission to access this asset');
					}//end else
				} else {
					$this->returnError('The createLink function must be activated');
				}//end else

			break;

			case	'removeLink':
				if ($this->attr('remove_link')) {
					if ($wa) {
						if (isset($_REQUEST['parent_id'])) {
							// Check to see if we are using an id or a URL
							$parent = $this->setAsset($_REQUEST['parent_id']);
							if (!$parent) return;
						} else {
							$this->returnError('A parent asset is required');
							return;
						}
						$link_type = defined($_REQUEST['link_type']) ? constant($_REQUEST['link_type']) : SQ_LINK_TYPE_1;
						$link_value = $_REQUEST['link_value'];
						// Remove our link
						$data = $this->removeLink($parent->id, $asset->id, $link_type, str_replace($replace_these, $replace_with, $link_value));
						if (!$data) return;
					} else {
						$this->returnError('You do not have permission to access this asset');
					}//end else
				} else {
					$this->returnError('The removeLink function must be activated');
				}//end else

			break;

			case	'moveLink':
				if ($this->attr('move_link')) {
					if ($wa) {
						if (!empty($_REQUEST['old_parent_id']) && !empty($_REQUEST['new_parent_id'])) {
							// Check to see if we are using an id or a URL
							$old_parent = $this->setAsset($_REQUEST['old_parent_id']);
							$new_parent = $this->setAsset($_REQUEST['new_parent_id']);
							if (is_null($old_parent) || is_null($new_parent)) {
								$this->returnError('Either Old Parent or the New Parent was not a valid assetid.');
								return;
							}
						} else {
							$this->returnError('A parent asset is required');
							return;
						}//end else

						$old_link_type = defined($_REQUEST['old_link_type']) ? constant($_REQUEST['old_link_type']) : SQ_LINK_TYPE_1 ;
						$old_link_value = str_replace($replace_these, $replace_with, $_REQUEST['old_link_value']);
						$new_link_type = defined($_REQUEST['new_link_type']) ? constant($_REQUEST['new_link_type']) : SQ_LINK_TYPE_1 ;
						$new_link_value = str_replace($replace_these, $replace_with, $_REQUEST['new_link_value']);
						$new_position = $_REQUEST['new_position'];

						// Remove our link
						$data = $this->moveLink($old_parent->id, $asset->id, $old_link_type, $old_link_value, $new_parent->id, $new_link_type, $new_link_value, $new_position);

						if (!$data) return;
					} else {
						$this->returnError('You do not have permission to access this asset');
					}//end else
				} else {
					$this->returnError('The moveLink function must be activated');
				}//end else

			break;

			case	'updateLink':
				if ($this->attr('update_link')) {
					if ($wa) {
						if (isset($_REQUEST['parent_id'])) {
							// Check to see if we are using an id or a URL
							$parent = $this->setAsset($_REQUEST['parent_id']);
							if (!$parent) return;
						} else {
							$this->returnError('A parent asset is required');
							return;
						}

						$old_link_type	= defined($_REQUEST['existing_link_type']) ? constant($_REQUEST['existing_link_type']) : SQ_LINK_TYPE_1 ; 
						$old_link_value	= str_replace($replace_these, $replace_with, $_REQUEST['existing_link_value']);
						$new_link_type	= defined($_REQUEST['link_type']) ? constant($_REQUEST['link_type']) : SQ_LINK_TYPE_1 ;
						$new_link_value	= str_replace($replace_these, $replace_with, $_REQUEST['link_value']);
						$sort_order		= (isset($_REQUEST['sort_order']) && $_REQUEST['sort_order'] != '' ) ?$_REQUEST['sort_order'] : NULL;
						$locked			= (isset($_REQUEST['locked']) && $_REQUEST['locked'] != '' ) ? $_REQUEST['locked'] : TRUE;

						// Remove our link
						$data = $this->updateLink($parent->id, $asset->id, $old_link_type, str_replace($replace_these, $replace_with,$old_link_value), $new_link_type,  str_replace($replace_these, $replace_with, $new_link_value), $sort_order, $locked);

						if (!$data) return;
					} else {
						$this->returnError('You do not have permission to access this asset');
					}//end else
				} else {
					$this->returnError('The updateLink function must be activated');
				}//end else

			break;

			case	'getLinkId':
				if ($this->attr('get_link_id')) {
					if ($ra) {
						if (isset($_REQUEST['parent_id'])) {
							// Check to see if we are using an id or a URL
							$parent = $this->setAsset($_REQUEST['parent_id']);
							if (!$parent) return;
						} else {
							$this->returnError('A parent asset is required');
							return;
						}
						$link_type = defined($_REQUEST['link_type']) ? constant($_REQUEST['link_type']) : SQ_LINK_TYPE_1 ;
						$link_value = $_REQUEST['link_value'];
						$all_info = (boolean)$_REQUEST['all_info'];
						// Get our link id
						$data = $this->getLinkId($parent->id, $asset->id, $link_type, str_replace($replace_these, $replace_with, $link_value), $all_info);
						if (!$data) {
							$data['error'] = 'Parent Asset #'.$parent->id.' and Child Asset #'.$asset->id.' do not have a valid link for given Link Type and Link Value';
						}
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end if
				} else {
					$this->returnError('The getLinkId function must be activated');
				}//end else

			break;

			case	'getAssetTree':
				if ($this->attr('get_asset_tree')) {
					if ($ra) {
						// Check to see how deep we should go
						if (empty($_REQUEST['depth'])) {
							$levels = NULL;
						} else {
							$levels = $_REQUEST['depth'];
						}//end if
						$data = $am->getAssetTree($asset->id, $levels);
						if (empty($data)) {
							$this->returnError('Asset #'.$asset->id.' has no children');
							return;
						}
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getChildren function must be activated');
				}//end else

			break;

			case	'getKeywordsReplacements':

				if ($this->attr('get_keywords_replacements')) {
					if ($ra) {
						if (isset($_REQUEST['keywords']) && !empty ($_REQUEST['keywords']) ){
							$keywords = explode('\\,', $_REQUEST['keywords']);
							$data = $this->getKeywordsReplacements($asset, $keywords);
						} else {
							$this->returnError('Please provide keywords to get replacement for');
						}
					} else {
						$this->returnError('You do not have permissions to access this asset');
					}//end else
				} else {
					$this->returnError('The getKeywordsReplacements function must be activated');
				}//end else
			break;

			case	'setAssetStatus':
				if ($this->attr('set_asset_status')) {
					if ($wa) {
						$status = (int) $_REQUEST['status'];
						$cascade = isset($_REQUEST['cascade']) ? $_REQUEST['cascade'] : FALSE;
						$workflow_stream = (isset($_REQUEST['workflow_stream']) && ($_REQUEST['workflow_stream'] != '') )? str_replace($replace_these, $replace_with, $_REQUEST['workflow_stream']) : 'SQ_USE_DEFAULT';

						//let check if the status supplied is valid one
						$desc = get_status_description($status);
						$current_desc = ($status != $asset->status) ? get_status_description($asset->status) : $desc;

						$response = $this->setAssetStatus($asset, $status, $cascade, $current_desc, $desc, $workflow_stream);
						if (empty($response)) {
							if (!$cascade) {
								$data[] = 'Status for Asset "'.$asset->name.'" (#'.$asset->id.') has been changed successfully to '.$desc;
							} else {
								$data[] = 'Status for Asset "'.$asset->name.'" (#'.$asset->id.') and its non-dependant children has been changed successfully to '.$desc;
							}
						} else {
							$data = $response;
						}

					} else {
						$this->returnError('You do not have permissions to access this change status for this asset');
					}//end else
				} else {
					$this->returnError('The setAssetStatus function must be activated');
				}//end else
			break;

			case	'getWebPath':

				if ($this->attr('get_web_path')) {
					if($ra) {
						$webpaths = $asset->getWebPaths();
						if(!empty($webpaths)) {
							$i = 0;
							foreach($webpaths as $webpath) {
								$data['webpath'][$i] = $webpath;
								$i++;
							}
						} else {
							$data ['webpath'] = Array();
						}
					} else {
						$this->returnError('You do not have permission to access this asset');
					}
				} else {
					$this->returnError('The getWebPath function must be activated');
				}
			break;

			case	'setWebPath':

				if ($this->attr('set_web_path')) {
					if($wa) {
						$webpaths = explode('\\,', $_REQUEST['webpath']);
						if (!empty($webpaths)) {
							$webpaths = explode('\\,', $_REQUEST['webpath']);
							foreach ($webpaths as $index => $webpath) {
								$webpaths[$index] = str_replace($replace_these, $replace_with, $webpath);
							}
							$auto_remap = (isset($_REQUEST['auto_remap']) && $_REQUEST['auto_remap'] != '') ? $_REQUEST['auto_remap'] : TRUE;
							$response = $asset->saveWebpaths($webpaths, $auto_remap);
							if($response) {
								$this->updateLookupsforAsset($asset->id);
								$data[] = 'Webpath for Asset "'.$asset->name.'" (# '.$asset->id.') had been updated sucessfully';
							} else {
								$data[] = 'Could not update webpath for Asset "'.$asset->name.'" (# '.$asset->id.') ';
							}
						} else {
							$this->returnError('No webpaths provided');
						}
					} else {
						$this->returnError('You do not have permission to access this asset');
					}
				} else {
					$this->returnError('The setWebPath function must be activated');
				}
			break;

			case	'getWorkflowSchema':

				if ($this->attr('get_workflow_schema')) {
					if($ra) {
						$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
						$granted = isset($_REQUEST['granted']) && $_REQUEST['granted'] != '' ? $_REQUEST['granted'] : NULL;
						$running = isset($_REQUEST['running']) && $_REQUEST['running'] != '' ? $_REQUEST['running'] : FALSE;
						$schemas = $wfm->getSchemas($asset->id, $granted, $running);
						if(empty($schemas)) {
							$message = 'No Workflow Schema ';
							if ($running) {
								$message .= 'currently running for ';
							} else {
								if (is_null($granted)) {
									$message .= 'currently applied to';
								} else {
									$message .= $granted ? 'granted on ' : 'denied on ';
								}
							}
							$data[] = $message.'Asset "'.$asset->name.'" (# '.$asset->id.')';
						} else {
							foreach ($schemas as $index => $info) {
								if ($running) {
									$data['running'][] = $info;
								} else {
									if (is_null($granted)) {
										$status = $info ? 'granted' : 'denied';
										$data[$status][] = $index;
									} else {
										$status = $granted ? 'granted' : 'denied';
										$data[$status][] = $info;
									}
								}
							}
						}
					} else {
						$this->returnError('You do not have permission to access this asset');
					}
				} else {
					$this->returnError('The getWorkflowSchema function must be activated');
				}
			break;

			case 'createFileAsset' :

                if($this->attr('create_asset'))  {
                    if ($wa || $ignore) {

						if ((isset($_REQUEST['friendly_name']) && $_REQUEST['friendly_name'] != '' )) {
							$filename		= $_REQUEST['friendly_name'];
							$type_code		= $_REQUEST['type_code'];
							$link_type		= defined($_REQUEST['link_type']) ? constant($_REQUEST['link_type']) : SQ_LINK_TYPE_1;
							$link_value		= $_REQUEST['link_value'];
							$valid_type_code= $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('file', TRUE);

							if(!in_array(Array($type_code), $valid_type_code)) {
								//We are going to write to the Matrix Data tmp dir
								$tmp_file_name  = $this->getRandomFilename($file_name);
								$file_ext = $this->get_file_extension($type_code);
								$destination_file   = SQ_DATA_PATH.'/temp/'.$tmp_file_name.'.'.$file_ext;

								while (file_exists($destination_file)) {
									$tmp_file_name  = $this->getRandomFilename($file_name);
									$destination_file   = SQ_DATA_PATH.'/temp/'.$tmp_file_name.'.'.$file_ext;
								}//end while

								//create an file with empty content, coz we are getting rid of it anyways
								if ($type_code != 'image' && $type_code != 'thumbnail') {
									file_put_contents($destination_file, '');
								} else {
									// is a image do it differently
									$img = imagecreate('10','10');
									imagepng($img, $destination_file);
								}

								// construct the link array 
								$link = Array (
											'asset'		=> $asset,
											'link_type'	=> $link_type,
											'value'		=> $link_value,
											'sort_order'=> -1,
										);

								$filename_w_extension = $filename.'.'.$file_ext;
								require_once SQ_FUDGE_PATH.'/general/file_system.inc';
								$file_type = get_file_type($filename_w_extension);

								$file_info = Array (
												'name'				=> $filename,
												'tmp_name'			=> $destination_file,
												'type'				=> $file_type,
												'error'				=> 0,
												'size'				=> filesize($destination_file),
												'filename'			=> $filename_w_extension,
												'non_uploaded_file' => TRUE,
											 );

								$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
								$new_asset = new $type_code();
								//fake it
								$new_asset->_tmp['uploading_file'] = TRUE;
								if ($type_code == 'data_source_graph') $new_asset->setAttrValue('title', $filename);
								$success = $new_asset->create($link, $file_info);

								if (!$success) {
									$data['error'] = 'Error occured while creating new asset of type \''.$type_code.'\'';
								} else {
									// done creating the asset ? let remove the temp file we had uploaded and make way for new one
									$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
									$existing = $new_asset->getExistingFile();

									$new_asset_edit_fns = $new_asset->getEditFns();
									$new_asset_edit_fns->removeOldFile($new_asset, $existing, $fv);
									$data[$new_asset->id] = 'New File Asset (#'.$new_asset->id.') \''.$new_asset->name.'\'of type_code \''.$type_code.'\' created successfully';
								}

							} else {
								$this->returnError('Passed type code is not File Type asset or its Descendant Type');
							}

						} else {
							$this->returnError('You must set the Name of the Asset and provide a Base 64 encoded file to upload');
						}
					} else {
						$this->returnError('You do not have permissions to create an asset');
					}

                } else {
                    $this->returnError('The createAsset function must be activated');
                }
			break;

			case 'setContentOfEditableFileAsset' :

				if($this->attr('set_file_content'))  {
					if ($wa) {
						$valid_file_type = Array('xml_file', 'xsl_file', 'css_file', 'text_file', 'js_file');
						if(in_array($asset->type(), $valid_file_type)) {
							if($asset->writeAccess('attributes')) {
								$content = urldecode($_REQUEST['content']);
								if($content == 'no_value_provided') {
									$data[] = 'No file content provided';
								} else {
									require_once(SQ_INCLUDE_PATH.'/backend_outputter.inc');
									$o			= new Backend_Outputter();
									$prefix 	= $asset->getPrefix();
									$edit_fns 	= $asset->getEditFns();
									$_POST[$prefix.'_new_file'] = urldecode($content);

									$success 	= $edit_fns->processEditFile($asset, $o, $prefix);
									if ($success) {
										$data['success'] = 'Content of Asset '.$asset->short_name.'(#'.$asset->id.') successfully updated';
									} else {
										$data['error'] = 'Error occured while updating Content of Asset '.$asset->short_name.'(#'.$asset->id.')';
									}
								}
							} else {
								$data['error'] = 'Locks are not acquired on Asset '.$asset->short_name.'(#'.$asset->id.'), content cannot be updated';
							}
						} else {
							$data['error'] = 'Content for Asset of type code \''.$asset->type().'\' cannot be updated';
						}
					} else {
						$this->returnError('You do not have permissions to Edit this asset');
					}
				} else {
					$this->returnError('The setContentOfEditableFileAsset function must be activated');
				}

			break;

			default:	$this->returnError('This is not a valid function name');

		}//end switch

		// Send our data as JSON
		if (isset($data)) {
			$this->returnJSON($data, $type);
		}//end if

		// lets restor PHP_SELF just incase if Matrix is going to check if $_SERVER var is
		// exactly the same when it went in THIS function and when i goes out
		$_SERVER['PHP_SELF'] = $temp_server_vars;

		// Restore error handler
		restore_error_handler();

	}//end printFrontend()


	/**
	* Description: This will create a link between two assets
	*
	* @param string		$id		The asset that we are getting
	*
	* @return object
	* @access public
	*/
	function setAsset($id)
	{
		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Check to see if the asset id is real
		if ($id == '') {
			$this->returnError('You must enter a valid asset id or URL');
			return FALSE;
		}

		// Check to see if we are using an id or URL
		if (is_numeric($id) || (preg_match('/^[0-9]*:/i', $id) != 0)) {

			// Check to see if the asset id is real
			try {
				$valid_assetid = $am->assetExists($id);
			} catch (Exception $e) {
				$this->returnError($id.' is not a valid asset id');
				return FALSE;
			}
			if (!$valid_assetid) {
				$this->returnError($id.' is not a valid asset id');
				return FALSE;
			}
			// Set our asset reference
			return $am->getAsset($id);

		} else {
			// We need to make sure this is a URL
			if (strpos($id, 'http://') !== FALSE || strpos($id, 'https://') !== FALSE) {
				// figure out if we are on dealing with a http or a https url here
				// results will be wrong here further if we let Matrix assume if the
				// protocol is always http
				$url_parts = parse_url($id);
				$protocol = (isset($url_parts['scheme']) && !empty($url_parts['scheme'])) ? $url_parts['scheme'] : 'http';

				// Set our asset reference
				$asset = $am->getAssetFromURL($protocol, strip_url($id, TRUE), TRUE, TRUE);
				if (is_null($asset)) $this->returnError($id.' is not a valid URL');
				return $asset;
			} else {
				$this->returnError($id.' is not a valid URL');
				return FALSE;
			}//end else

		}//end else

	}//end setAsset


	/**
	* Gets general information for the passed in Asset
	*
	* @param object		$asset					The asset we want to get information about
	* @param boolean	$add_more_attributes	if we are getting additional attributes and its values for the asset(FALSE by default)
	*
	* @return array
	* @access public
	*/
	public function getGeneralInfo(Asset $asset, $add_more_attributes=FALSE)
	{
		if (is_null($asset)) return Array();

		//root folder?
		if ($asset->id == '1') {
			$data['name'] = 'Root Folder';
			return $data;
		}

		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		if (isset($asset->created_userid) && $asset->created_userid != '' && (int)$asset->created_userid != 0 ) {
			if($am->assetExists($asset->created_userid)) {
				$created_user = $am->getAsset($asset->created_userid);
			} else {
				$created_user = 'Unknown User';
			}
		} else if (isset($asset->created_userid) && (int)$asset->created_userid == 0) {
			$created_user = 'Root User';
		} else {
			$created_user = 'Undefined';
		}

		if (isset($asset->updated_userid) && $asset->updated_userid != '' && (int)$asset->updated_userid != 0 ) {
			if($am->assetExists($asset->updated_userid)) {
				$updated_user = $am->getAsset($asset->updated_userid);
			} else {
				$updated_user = 'Unknown User';
			}
		} else {
			$updated_user = 'Never Updated';
		}

		if (isset($asset->published_userid) && $asset->published_userid != '' && (int)$asset->published_userid != 0 ) {
			if($am->assetExists($asset->published_userid)) {
				$published_user = $am->getAsset($asset->published_userid);
			} else {
				$published_user = 'Unknown User';
			}
		} else {
			$published_user = 'Never Published';
		}

		if (isset($asset->status_changed_userid) && $asset->status_changed_userid != '' && (int)$asset->status_changed_userid != 0 ) {
			if($am->assetExists($asset->status_changed_userid)) {
				$status_user = $am->getAsset($asset->status_changed_userid);
			} else {
				$status_user = 'Unknown User';
			}
		} else {
			$status_user = 'Never Changed';
		}

		$user = $am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
		$array_to_check_in = array_merge(Array($user->id), $user->getUserGroups());
		$effective_permission = '';

		if($user instanceof Root_User || $user instanceof System_User) {
			$effective_permission = 'Admin Access';
		}

		//if we havent got max permissions yet try the user id and all the user groups it belongs in
		if($effective_permission === '' ) {
			foreach($array_to_check_in as $user_group_id) {
				if ($effective_permission != '' ) continue;
				if ($asset->adminAccess('', Array($user_group_id))) {
					$effective_permission = 'Admin Access';
				}else if($asset->writeAccess('', Array($user_group_id), FALSE)) {
					$effective_permission = 'Write Access';
				} else if($asset->readAccess(Array($user_group_id))) {
					$effective_permission = 'Read Access';
				}
			}
		}

		// Get general info about asset
		
		$data = Array(
					'name'						=>	$asset->name,
					'short_name'				=>	$asset->short_name,
					'id'						=>	$asset->id,
					'type_code'					=>	$asset->type(),
					'type'						=>	str_replace('_', ' ', get_class($asset)),
					'icon_path'					=>	$am->getAssetIconURL($asset->type()),
					'data_path'					=>	$asset->data_path,
					'web_path'					=>	$asset->getURL(),
					'status'					=>	$asset->getStatusDescription(),
					'created'					=>	$asset->created,
					'created_userid'			=>	$asset->created_userid,
					'created_username'			=>	($created_user === 'Root User' || $created_user === 'Undefined' || $created_user === 'Unknown User') ? $created_user : $created_user->name,
					'updated'					=>	($updated_user === 'Never Updated' || $update_user === 'Unknown User') ? $updated_user : $asset->updated,
					'updated_userid'			=>	($updated_user === 'Never Updated' || $update_user === 'Unknown User') ? $updated_user : $updated_user->id,
					'update_username'			=>	($updated_user === 'Never Updated' || $update_user === 'Unknown User') ? $updated_user : $updated_user->name,
					'published'					=>	($published_user === 'Never Published' || $published_user === 'Unknown User') ? $published_user : $asset->published,
					'published_userid'			=>	($published_user === 'Never Published' || $published_user === 'Unknown User') ? $published_user : $published_user->id,
					'published_username'		=>	($published_user === 'Never Published' || $published_user === 'Unknown User') ? $published_user : $published_user->name,
					'status_changed'			=>	($status_user === 'Never Changed' || $status_user === 'Unknown User') ? $status_user : $asset->status_changed,
					'status_changed_userid'		=>	($status_user === 'Never Changed' || $status_user === 'Unknown User') ? $status_user : $status_user->id,
					'status_changed_username'	=>	($status_user === 'Never Changed' || $status_user === 'Unknown User') ? $status_user : $status_user->name,
					'maximum_perm_on_asset'		=>	$effective_permission,
				);

		if($add_more_attributes) {
			foreach($am->getAssetTypeAttributes($asset->type()) as $name => $type) {
				$value = $asset->attr($name);
				if(!empty($value) && !is_null($value) && !isset($data[$name])) {
					$data[$name] = $value;
				}
			}
		}

		return $data;

	}//end getGeneralInfo()


	/**
	* Description: This will create a link between two assets
	*
	* @param integer	$parent			Major asset id we are linking from
	* @param integer	$child			Minor asset id we are linking to
	* @param integer	$link_type		Type of link to create
	* @param string		$link_value		Value of the link
	* @param integer	$sort_order		Order in the tree
	* @param integer	$is_dependant	Dependant to parent
	* @param integer	$is_exclusive	Exclusive to parent
	*
	* @return array
	* @access public
	*/
	function createLink($parent, $child, $link_type, $link_value, $sort_order, $is_dependant, $is_exclusive)
	{
		if (!empty($parent) && !empty($child)) {
			// Set some shortcuts
			$am = $GLOBALS['SQ_SYSTEM']->am;

			$parent	= $am->getAsset($parent);
			$child	= $am->getAsset($child);
			$new_id = $am->createAssetLink($parent, $child, $link_type, $link_value, $sort_order, $is_dependant, $is_exclusive);
			//done creating the link, lets update lookup
			$result = $this->updateLookupsforAsset($child->id);
			if (!is_null($result)) $this->returnError('Unable to Update Lookups on the Asset #'.$child->id.' and its Childrens');

			if ($new_id == 0) {
				$this->returnError('Unable to create link');
			}//end if
			return Array (
					'link_id'	=>	$new_id,
				   );
		} else {
			$this->returnError('Missing information to create link. Make sure parent id and child are provided');
		}//end else

	}//end CreateAssetLink()


	/**
	* Description: This operation will delete a link based on the LinkID
	*
	* @param string		$parent			Major asset id we are linking from
	* @param string		$child			Minor asset id we are linking to
	* @param string		$link_type		Link type between parent and child asset
	* @param string		$link_value		Link value of link between parent and child asset
	*
	* @return array
	* @access public
	*/
	function removeLink($parent, $child, $link_type=NULL, $link_value=NULL)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Set our link id
		$linkid = $this->getLinkId($parent, $child, $link_type, $link_value, TRUE);

		if (!$linkid) {
			$data['error'] = 'Parent Asset #'.$parent.' and Child Asset #'.$child.' do not have a valid link for given Link Type and Link Value';
			return $data;
		}

		$parent_asset = $am->getAsset($parent);
		$child_asset = $am->getAsset($child);

		if ($linkid['locked']) {
			$data['error'] = 'Link #"'.$linkid['link_id'].'" between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.') " cannot be deleted, link is locked';
			return $data;
		}

		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		if ($linkid['link_id'] != '') {
			$link = $am->getLinkById($linkid['link_id']);
			// Make sure our array contains some items
			if (!empty($link)) {
				// Get all our locks
				$am->acquireLock($link['majorid'], 'all');
				$am->acquireLock($link['minorid'], 'all');
				$parents = $am->getLinks($link['minorid'], SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');

				// See if we should delete a link or move the asset to the trash
				// but do this only if we are deleting a SIGNIFICANT LINK and not NOTICE link
				if (count($parents) > 1 || !($link_type & SQ_SC_LINK_SIGNIFICANT)) {
					// More than one parent link, we can delete the link
					$result	= $am->deleteAssetLink($linkid['link_id']);
				} else {
					// Only one link holly cow, lets trash it instead, otherwise we'l have an orphan asset in the system
					$result	= $am->trashAsset($link['minorid']);
				}//end if
				//no matter if link is deleted or the whole asset is moved to thrash, we will update lookups :)
				$result_from_update = $this->updateLookupsforAsset($child);
				if (!is_null($result_from_update)) $this->returnError('Unable to Update Lookups on the Asset "'.$child_asset->name.'" (#'.$child.') and its Childrens');

				// Release all our locks
				$am->releaseLock($link['minorid'], 'all');
				$am->releaseLock($link['majorid'], 'all');

				if ($result) {
					$data['success'] = 'Link #"'.$linkid['link_id'].'" between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.') " has been sucessfully removed';
					return $data;
				} else {
					$this->returnError('Unable to delete link');
				}//end else

			} else {
				$this->returnError('The link id that was provided is invalid');
			}//end else

		} else {
			$this->returnError('The link id provided is empty');
		}//end else

	}//end removeLink


	/**
	* Description: This operation will move a link under a new parent asset
	*
	* @param integer	$old_parent_id		Old parent id
	* @param integer	$asset_id_to_move	Asset id to move
	* @param constant	$old_link_type		Link type between asset and existing parent
	* @param string		$old_link_value		Link value between asset and existing parent
	* @param integer	$new_parent_id		New parent id
	* @param integer	$old_link_type		Link type to create on moving the asset
	* @param integer	$old_link_value		Link value to create on moving the asset
	* @param integer	$new_position		Position under the new parent
	*
	* @return void
	* @access public
	*/
	function moveLink($old_parent_id, $asset_id_to_move, $old_link_type=SQ_LINK_TYPE_1, $old_link_value='', $new_parent_id, $new_link_type=SQ_LINK_TYPE_1, $new_link_value='', $new_position)
	{
		// lets make sure the link type passed in for both, old link and new link are significant
		if (!($new_link_type & SQ_SC_LINK_SIGNIFICANT)) $new_link_type = SQ_LINK_TYPE_1;
		if (!($old_link_type & SQ_SC_LINK_SIGNIFICANT)) $old_link_type = SQ_LINK_TYPE_1;

		if (!empty($old_parent_id) && !empty($asset_id_to_move) && !empty($new_parent_id)) {
			// Get our link id!!
			$link_id = $this->getLinkId($old_parent_id, $asset_id_to_move, $old_link_type, $old_link_value, TRUE);

			if (!$link_id) {
				$data['error'] = 'Parent Asset #'.$old_parent_id.' and Child Asset #'.$asset_id_to_move.' do not have a valid link for given Link Type and Link Value';
				return $data;
			}

			if ($link_id['locked']) {
				$this->returnError('Link "'.$link_id['link_id'].'" cannot be moved, link is locked');
				return FALSE;
			}
			$new_link_id = $GLOBALS['SQ_SYSTEM']->am->moveLink($link_id['link_id'], $new_parent_id, $new_link_type, $new_position, $new_link_value);

			if (!empty($new_link_id)) {
				//wow we moved the asset successfully, now lets try and update lookups
				$result = $this->updateLookupsforAsset($asset_id_to_move);
				if (!is_null($result)) $this->returnError('Unable to Update Lookups on the Asset #'.$asset_id_to_move.' and its Childrens');
				return Array (
						'link_id'	=> $new_link_id,
					   );
			} else {
				$this->returnError('Unable to move link');
			}//end else
		} else {
			$this->returnError('Please provide a valid current parent id, asset id to move, and new parent id');
		}//end else

	}//end moveLink


	/**
	* Description: This operation will update an existing link
	*
	* @param integer	$parent			Major asset id we are linking from
	* @param integer	$child			Minor asset id we are linking to
	* @param integer	$link_type		Type of link to set
	* @param string		$link_value		Value of the link
	* @param integer	$sort_order		Sort order in the asset tree
	* @param integer	$locked			the asset's link lock status
	*
	* @return array
	* @access public
	*/
	function updateLink($parent, $child, $existing_link_type=SQ_LINK_TYPE_1, $existing_link_value= '', $link_type=SQ_LINK_TYPE_1, $link_value='', $sort_order, $locked=TRUE)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		if (!empty($parent) && !empty($child)) {

			$link_id = $this->getLinkId($parent, $child, $existing_link_type, $existing_link_value, TRUE);
			$parent_asset = $am->getAsset($parent);
			$child_asset = $am->getAsset($child);

			if (!$link_id) {
				$data['error'] = 'Parent Asset #'.$parent.' and Child Asset #'.$child.' do not have a valid link for given Link Type and Link Value';
				return $data;
			}

			// we cannot change a link from being a significant link to a non-significant link or vice versa
			// so if we are try to do that, do what a backend does, delete the old one and then create a new
			// one with all same properties except for changed linked value
			$current_is_sig = (bool) ((int) $link_id['link_type'] & SQ_SC_LINK_SIGNIFICANT);
			$new_is_sig     = (bool) ($link_type & SQ_SC_LINK_SIGNIFICANT);
			if ($current_is_sig !== $new_is_sig) {
				if ($am->deleteAssetLink($link_id['link_id'], FALSE)) {
					// irrespective of what is passed value of link lock, if the link isnt siginificant, then dont bother locking it
					if (!$new_is_sig) {
						$lock = 0;
					} else {
						$lock = $locked;
					}
					$new_link_id = $am->createAssetLink($parent_asset, $child_asset, $link_type, $link_value, $sort_order, $link_id['is_dependant'], $link_id['is_exclusive'], FALSE, $lock);
				}
			
			} else {
				$new_link_id = $am->updateLink($link_id['link_id'], $link_type, $link_value, $sort_order, $locked);
			}

			if (!empty($new_link_id)) {
				$response['success'] = 'Link #'.$link_id['link_id'].' between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.') has been updated';
				return $response;
			} else {
				$response['error'] = 'Unable to update Link #'.$link_id['link_id'].' between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.')';
				return $response;
			}//end else
		} else {
			$this->returnError('Please enter a valid parent id and child id');
			return FALSE;
		}//end else

	}//end updateLink


	/**
	* Description: This operation will return the permission set for on an asset
	*
	* @param integer	$id				Id of the asset to get permissions for
	* @param string		$level			Read, Write, Admin
	*
	* @return void
	* @access public
	*/
	function getPermission($id, $level)
	{
		if (!empty($id) && !empty($level)) {
			// Shortcuts
			$am = $GLOBALS['SQ_SYSTEM']->am;

			// Set our level
			if ($level == 1) {
				$level = SQ_PERMISSION_READ;
				$level_name = 'read';
			} else if ($level == 2) {
				$level = SQ_PERMISSION_WRITE;
				$level_name = 'write';
			} else if ($level == 3) {
				$level = SQ_PERMISSION_ADMIN;
				$level_name = 'admin';
			}//end else if

			$permissions = $am->getPermission($id, $level, NULL, FALSE);
			if (empty($permissions)) {
				$this->returnError('There are no '.$level_name.' permissions set for id #'.$id);
				return FALSE;
			}

			foreach ($permissions as $key) {
				$asset = $am->getAsset($key);

				$data[$key]['id'] = $key;
				$data[$key]['name'] = $asset->name;
				if (isset($asset->vars['username'])) {
					$data[$key]['user_name'] = $asset->attr('username');
				}
				$data[$key]['type'] = $asset->type();

				// Expand our user groups
				if ($asset->type() == 'user_group') {

					$data[$key]['users'] = $this->getGroupChildren($key);

				}//end if

			}//end foreach

			return $data;

		} else {
			$this->returnError('Asset ID or Access Level is not valid. Please provide a valid AssetID and Access Level');
			return FALSE;
		}//end else

	}//end getPermission


	/**
	* Description: Returns arrays of user assets from a parent user group
	*
	* @param integer	$key			Asset id of the user group
	*
	* @return void
	* @access public
	*/
	function getGroupChildren($key)
	{
		// Shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$group_children = $am->getChildren($key, '', TRUE, NULL, NULL, NULL, TRUE, NULL, 1);
		foreach ($group_children as $children => $child_id) {
			$child = $am->getAsset($children);
			$children_arr[$child->id] = Array(
							'id'		=>	$child->id,
							'name'		=>	$child->name,
							'type'		=>	$child->type(),
							'username'	=>	$child->attr('username'),
						);
		}//end foreach

		return $children_arr;

	}//end getGroupChildren


	/**
	* Description: Gets parent asset ids
	*
	* @param integer	$id					Asset id of the child
	* @param integer    $level				How deep to go
	* @param array		$type_codes			type_code of assets to return
	* @param array		$link_types			type of link we are looking to return
	* @param array		$link_values		link values we want on the assets returned
	* @param boolean	$also_attributes	if we are getting additional attributes and its values(FALSE by default)
	*
	* NOTE : link_types and link_values works only if the level of gettign the child assets is set to 1
	*
	* @return void
	* @access public
	*/
	function getParents($id, $level, $type_codes=Array(), $link_types=Array(), $link_values=Array(), $also_attributes=FALSE)
	{
		// Shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$parent_arr = Array();

		$parents = $am->getParents($id, '', TRUE, NULL, NULL, TRUE, NULL, $level);
		foreach ($parents as $parent => $type_code) {
			$parent_asset = $am->getAsset($parent);
			// not in the type_code we want, remove it
			if(!empty($type_codes) && !in_array($parent_asset->type(), $type_codes)) {
				unset($childrens[$children]);
				continue;
			}

			// Levels greater than 1 won't have a link id with the parent
			if (!$this->hasLink($parent, $id)) {
				$show['link_id']		= NULL;
				$show['link_type']		= NULL;
				$show['is_dependant']	= NULL;
				$show['is_exclusive']	= NULL;
				$show['sort_order']		= NULL;
				$show['link_value']		= NULL;
				$show['locked']			= NULL;
			} else {
				$show_link = $this->getLinkId($parent, $id, NULL, NULL, TRUE);
				if (!$show_link) {
					$data['error'] = 'Parent Asset #'.$parent.' and Child Asset #'.$id.' do not have a valid link';
					return $data;
				}

				// abra-ka-dabra all except for the link_values we are interested in
				if (!empty($link_values) && !in_array($show_link['value'], $link_values)) {
					unset($childrens[$children]);
					continue;
				}

				if(!empty($link_types)) {
					$found = FALSE;
					foreach($link_types as $link_type) {
						if(defined($link_type) && (constant($link_type) == $show_link['link_type'])) {
							$found = TRUE;
						} else if (!defined($link_type)) {
							$found = TRUE;
						}
					}
					if(!$found) {
						unset($childrens[$children]);
						continue;
					}
				}

				$show['link_id']		= $show_link['link_id'];
				$show['link_type']		= $show_link['link_type'];
				$show['is_dependant']	= $show_link['is_dependant'];
				$show['is_exclusive']	= $show_link['is_exclusive'];
				$show['sort_order']		= $show_link['sort_order'];
				$show['link_value']		= $show_link['value'];
				$show['locked']			= $show_link['locked'];
			}
			$parent_arr[$parent_asset->id] = array_merge($this->getGeneralInfo($parent_asset, $also_attributes), $show);

		}//end foreach

		return $parent_arr;

	}//end getParents


	/**
	* Description: Gets child asset ids
	*
	* @param integer	$id					Asset id of the parent
	* @param integer	$level				How deep to go
	* @param array		$type_codes			type_code of assets to return
	* @param array		$link_types			type of link we are looking to return
	* @param array		$link_values		link values we want on the assets returned
	* @param boolean	$also_attributes	if we are getting additional attributes and its values(FALSE by default)
	*
	* NOTE : link_types, type_codes and link_values works only if the level of gettign the child assets is set to 1
	*
	* @return void
	* @access public
	*/
	function getChildren($id, $level, $type_codes=Array(), $link_types=Array(), $link_values=Array(), $also_attributes=FALSE)
	{
		// Shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$children_arr =Array();

		$childrens = $am->getChildren($id, '', TRUE, NULL, NULL, NULL, TRUE, NULL, $level);
		foreach ($childrens as $children => $child_id) {
			$child = $am->getAsset($children);
			// not in the type_code we want, remove it
			if(!empty($type_codes) && !in_array($child->type(), $type_codes)) {
				unset($childrens[$children]);
				continue;
			}

			// Levels lower than 1 won't have a link id with the parent
			if (!$this->hasLink($id, $children)) {
				$show['link_id']		= NULL;
				$show['link_type']		= NULL;
				$show['is_dependant']	= NULL;
				$show['is_exclusive']	= NULL;
				$show['sort_order']		= NULL;
				$show['link_value']		= NULL;
				$show['locked']			= NULL;
			} else {
				$show_link = $this->getLinkId($id, $child->id, NULL, NULL, TRUE);
				if (!$show_link) {
					$data['error'] = 'Parent Asset #'.$id.' and Child Asset #'.$child->id.' do not have a valid link';
					return $data;
				}

				// abra-ka-dabra all except fo the link_values we are interested in
				if (!empty($link_values) && !in_array($show_link['value'], $link_values)) {
					unset($childrens[$children]);
					continue;
				}

				if(!empty($link_types)) {
					$found = FALSE;
					foreach($link_types as $link_type) {
						if(defined($link_type) && (constant($link_type) == $show_link['link_type'])) {
							$found = TRUE;
						} else if (!defined($link_type)) {
							$found = TRUE;
						}
					}
					if(!$found) {
						unset($childrens[$children]);
						continue;
					}
				}

				$show['link_id']		= $show_link['link_id'];
				$show['link_type']		= $show_link['link_type'];
				$show['is_dependant']	= $show_link['is_dependant'];
				$show['is_exclusive']	= $show_link['is_exclusive'];
				$show['sort_order']		= $show_link['sort_order'];
				$show['link_value']		= $show_link['value'];
				$show['locked']			= $show_link['locked'];
			}

			$direct_dependant_children = $am->getLinks($child->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'major', NULL, TRUE);
			$show['direct_dependant_children'] = count($direct_dependant_children);

			$direct_non_dependant_children = $am->getLinks($child->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'major', NULL, FALSE);
			$show['direct_non_dependant_children'] = count($direct_non_dependant_children);

			$direct_shadow_children = $am->getLinks($child->id, SQ_LINK_NOTICE, '', TRUE, 'major', NULL, FALSE);
			$show['direct_notice_linked_children'] = count($direct_shadow_children);

			$children_arr[$child->id] = array_merge($this->getGeneralInfo($child, $also_attributes), $show);

		}//end foreach

		return $children_arr;

	}//end getChildren


	/**
	* Description: This operation will send an asset to the trash
	*
	* @param array  $ids  The id of the assets we want to delete
	*
	*
	* @return array
	* @access public
	*/
	function trashAsset($ids)
	{
		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$response = Array();

		if (!empty($ids)) {
			foreach($ids as $id) {
				$already_in_trash = $am->assetInTrash($id, TRUE);
				$asset = $am->getAsset($id);
				//need to make sure that links are not locked
				$parents = $this->getParents($id, 1);
				$locked = FALSE;
				//loop through each parents and see if any of the links are locked
				foreach ($parents as $parentid => $parent_link_info) {
					if($parent_link_info['locked'] == '1') $locked = TRUE;
				}

				if ($already_in_trash) {
					$response[] = 'Asset "'.$asset->name.'" (#'.$asset->id.') is already in the trash';
				} else if ($locked && $asset->writeAccess()){
					// few or all the links are locked so lets go through each one of them and try to delete the links
					foreach ($parents as $parentid => $parent_link_info) {
						$parent_asset = $am->getAsset($parentid);
						if ($parent_link_info['locked'] == '1') {
							// found the link locked.... 
							$response[] = 'Link between Asset "'.$asset->name.'" (#'.$asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.') is locked';
						} else {
							//link not locked....lets get it done with
							if($am->deleteAssetLink($parent_link_info['link_id'])) {
								$response[] = 'Successfully unlinked link between Asset "'.$asset->name.'" (#'.$asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.')';
							}
						}
					}
				} else if ($asset->writeAccess()) {
					// asset isnt in trash and none of the links are locked, Hooorrrrray !!!!
					$am->acquireLock($id, 'all');
					$result	= $am->trashAsset($id);
					if (!$result) {
						$response[] = 'Could not move Asset "'.$asset->name.'" (#'.$asset->id.') to Trash';
					} else {
						//done thrashing the asset? update lookups then
						$result_from_update = $this->updateLookupsforAsset($id);
						if (!is_null($result_from_update)) {
							$response[] = 'Asset deleted but unable to Update Lookups on the Asset "'.$asset->name.'" (#'.$asset->id.')';
						} else {
							//all good? let the user know what we did
							$response[] = 'Asset "'.$asset->name.'" (#'.$asset->id.') successfully moved to Trash';
						}
					}
					$am->releaseLock($id, 'all');
				} else {
					$response[] = 'Not enough permision to delete Asset "'.$asset->name.'" (#'.$asset->id.')';
				}

			}
		} else {
			$this->returnError('Please provide a valid Asset ID');
			return;
		}//end else

		return $response;

	}//end trashAsset()


	/**
	* Description: This operation will return set the attribute value of an asset based on assetid and attribute name
	*
	* @param string		$assetid	The ID of the asset in query
	* @param array		$attr_name	The names of the attributes
	* @param array		$attr_val	The new values of the attributes
	*
	* @return array
	* @access public
	*/
	function setAttributeValue($assetid, $attr_name, $attr_val)
	{
		if (!empty($assetid) && !empty($attr_name) && !empty($attr_val)) {
			if (count($attr_name) != count($attr_val) ) $this->returnError('Please provide Attributes and corresponding values correctly');
			// Shortcuts
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$am = $GLOBALS['SQ_SYSTEM']->am;

			// Change our values
			$am->acquireLock($assetid, 'attributes');
			for ($i = 0; $i < count($attr_name); $i++ ) {
				$attributes = $am->getAssetTypeAttributes($asset->type());
				if ($attr_name[$i] != '' && array_key_exists($attr_name[$i], $attributes)) {
					//set our new attribute
					$asset->setAttrValue($attr_name[$i], $attr_val[$i]);
				} else if (!array_key_exists($attr_name[$i], $attributes)){
					$data[] = 'Attribute "'.$attr_name[$i].'" does not exist for Asset "'.$asset->name.'" (#'.$assetid.')';
					continue;
				}
				if (!$asset->saveAttributes()) {
					$data[] = 'Attribute "'.$attr_name[$i].'" could not be set to "'.$attr_val[$i].'" for Asset "'.$asset->name.'" (#'.$assetid.')';
				} else {
					$data[] = 'Attribute "'.$attr_name[$i].'" has been successfully set to "'.$attr_val[$i].'" for Asset "'.$asset->name.'" (#'.$assetid.')';
				}
			}//end for
			$am->releaseLock($assetid, 'attributes');
			$am->forgetAsset($asset);
			return $data;
		} else {
			$this->returnError('Please make sure to provide an Asset ID, Attribute Name and Attribute Value');
		}//end else

	}//end setAttributeValue()


	/**
	* Description: This operation will set the value for a metadata field(s) of an asset
	*
	* @param string		$assetid	The asset to regenerate metadata for
	* @param array		$fieldid	Array of the metadata field ids
	* @param array		$new_value	Array of the new value corresponding to metadata field
	*
	* @return void
	* @access public
	*/
	function setAssetMetadata($assetid, $fieldid, $new_value)
	{
		if (!empty($assetid) && !empty($fieldid)) {
			if (count($fieldid) != count($new_value) ) $this->returnError('Please provide Metadata fieldids and corresponding values correctly');
			$s_result = TRUE;
			$g_result = TRUE;

			// Shortucts
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$am = $GLOBALS['SQ_SYSTEM']->am;

			$am->acquireLock($assetid, 'metadata');
			// All context metadata files need to be regenerated if there is a non-contextable field changed
			$regen_all_contexts = FALSE;
			for ($i = 0; $i < count($fieldid); $i++ ) {

				$field	= $am->getAsset($fieldid[$i]);
				if (!($field instanceof Metadata_Field)) {
					$this->returnError('The Field ID provided does not belong to a metadata field');
				}//end if

				if (!$regen_all_contexts && (!$field->attr('is_contextable') || $field instanceof Metadata_Field_Select)) {
					$regen_all_contexts = TRUE;
				}
				
				$field_name		= $field->attr('name');
				$metadata_info	= Array (
									$fieldid[$i]	=> Array (
														Array (
															'name'	=> $field_name,
															'value'	=> $new_value[$i],
														),
													   ),
								  );
				// Set metadata
				if (!$mm->setMetadata($assetid, $metadata_info)) $s_result = FALSE;

			}//end for
			$am->releaseLock($assetid, 'metadata');

			// if the Metadata wasnt set correctly, dont even worry about regenerating the metadata file
			// just return from here telling the user it was a big time fail ;)
			if(!$s_result) {
				$this->returnError('Unable to set Matadata for Asset #'.$assetid);
				return FALSE;
			}

			// instead for doing it for every field, lets regen content file just once here
			if ($regen_all_contexts) {
				$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
				foreach ($all_contexts as $contextid => $context_data) {
					if (!$mm->generateContentFile($assetid, FALSE, $contextid)) {
						$g_result = FALSE;
						break;
					}
				}
			} else {
				if (!$mm->generateContentFile($assetid)) $g_result = FALSE;
			}

			if ($s_result && $g_result) {
				return TRUE;
			} else {
				$this->returnError('Unable To Regenerate Metadata For Asset #'.$assetid);
				return FALSE;
			}//end else
		} else {
			$this->returnError('Please provide both AssetID and FieldID of the metadata field');
		}//end else

	}//end setAssetMetadata()


	/**
	* Description: Returns the link id between a parent and a child
	*
	* @param string		$parent			Parent id
	* @param string		$child			Child id
	* @param string		$link_type		Type of link we are looking for (default to TYPE 1 links)
	* @param string		$link_value		Value of the link we are looking for(Default to NULL)
	* @param boolean	$all_info		If to return just LinkId or all the information
	*
	* @return array
	* @access public
	*/
	function getLinkId($parent, $child, $link_type=SQ_LINK_TYPE_1, $link_value=NULL, $all_info = FALSE)
	{
		if (!empty($parent) && !empty($child)) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent, $child, $link_type, $link_value, 'major', TRUE);
			if (empty($link)) {
				return FALSE;
			} else if (count($link) > 1) {
				$this->returnError('More than one link found between Parent Asset #'.$parent.' and Child Asset #'.$child.', please filter out with help of Link value and Link Type');
				return FALSE;
			}

			$link = $link[0];
			if (!$all_info) {
				$data_array['link_id'] = array_get_index($link, 'linkid', 0);
				return $data_array;
			} else {
				// we are returning more information
				$data_array = Array (
								'link_id'		=> isset($link['linkid']) ? $link['linkid'] : '',
								'minorid'		=> isset($link['minorid']) ? $link['minorid'] : '',
								'majorid'		=> $parent,
								'value'			=> isset($link['value']) ? $link['value'] : '',
								'link_type'		=> isset($link['link_type']) ? $link['link_type'] : '',
								'sort_order'	=> isset($link['sort_order']) ? $link['sort_order'] : '',
								'is_dependant'	=> isset($link['is_dependant']) ? $link['is_dependant'] : '',
								'is_exclusive'	=> isset($link['is_exclusive']) ? $link['is_exclusive'] : '',
								'locked'		=> isset($link['locked']) ? $link['locked'] : '',
							  );
				return $data_array;
			}
		} else {
			$this->returnError('Please provide a parent id and child id');
			return FALSE;
		}//end else

	}//end getLinkId


	/**
	* Description: Check to see if there is a link id between a parent and child
	*
	* @param string		$parent			Parent id
	* @param string		$child			Child id
	*
	* @return void
	* @access public
	*/
	function hasLink($parent, $child)
	{
		if (!empty($parent) && !empty($child)) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent, $child);
			$linkid = array_get_index($link, 'linkid', 0);
			if (!empty($linkid)) {
				// Found a link
				return TRUE;
			}//end if
		}//end if
	
		return FALSE;

	}//end hasLink


	/**
	* Description: This operation will create an asset of a specific type under a specific location
	*
	* @param integer	$id					Parentid of the new parent
	* @param string		$type_code			Type code of new asset
	* @param string		$asset_name			Name for new asset
	* @param integer	$link_type			Type of link to create
	* @param string		$link_value			Value of the link
	* @param integer	$sort_order			Order in the tree
	* @param integer	$is_dependant		Dependant to parent
	* @param integer	$is_exclusive		Exclusive to parent
	* @param boolean	$extra_attributes	Any extra attributes to create
	*
	* @return void
	* @access public
	*/
	function createAsset($id, $type_code, $asset_name, $link_type=1, $link_value, $sort_order, $is_dependant=0, $is_exclusive=0, $extra_attributes=FALSE)
	{
		// Shortucts
		$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$am = $GLOBALS['SQ_SYSTEM']->am;
		// Get our parent asset
		$pa = $am->getAsset($id);
		// Set our asset type to create
		$type_code = strtolower($type_code);
		if (!$am->installed($type_code)) {
			return Array();
		}//end if

		// Are we allowed to create this?
		$types = $this->attr('types');
		if (!empty($types)) {
			if (!isset($types[$type_code])) {
				return FALSE;
			}
		}

		// Are we allowed to create this asset under this parent type?
		$restrict = $this->attr('types_restriction');
		if (!empty($restrict)) {
			if (!$this->checkParentType($pa)) {
				return FALSE;
			}
		}
		
		$am->includeAsset($type_code);

		$object_name = str_replace(' ', '_', ucwords(str_replace('_', ' ', $type_code)));
		$asset = new $object_name();
		$asset->setAttrValue('name', $asset_name);
		$request_info['asset']	= $pa;
		// Set our asset info
		$link_info	= Array (
						'asset'			=> $pa,
						'name'			=> $asset_name,
						'link_type'		=> $link_type,
						'value'			=> $link_value,
						'sort_order'	=> $sort_order,
						'is_dependant'	=> $is_dependant,
						'is_exclusive'	=> $is_exclusive,
					  );

		// Should we add some attributes to our new asset?
		if ($extra_attributes && $this->attr('create_attributes')) {
			// Check our post values for any correct attibutes
			$attributes = $am->getAssetTypeAttributes($type_code);
			foreach($_REQUEST as $name => $value) {
				if (array_key_exists($name, $attributes) && $name !== $link_info[$name]) {
					// Set our new values
					$asset->setAttrValue($name, $value);
				}//end if
			}//end foreach
		}//end if

		$link_id = $asset->create($link_info);

		if ($link_id) {
			// Save and quit
			$am->acquireLock($asset->id, 'attributes');
			$asset->saveAttributes();
			$am->releaseLock($asset->id, 'attributes');
			return Array (
					'name'		=>	$asset_name,
					'id'		=>	$asset->id,
					'link_id'	=>	$link_id,
				   );
		}//end if

		return Array();

	}//end createAsset


	/**
	* Description: Check out see if the parent passed matches the allowed types
	*
	* @param object	 $parent	The asset to check
	*
	* @return TRUE
	* @access public
	*/
	function checkParentType($parent)
	{
		// Are we allowed to create this asset under this parent type?
		if (array_key_exists($parent->type(), $this->attr('types_restriction'))) {
			return TRUE;
		}

	}//end checkParentType
	
	
	/**
	* Description: Check out see if the passed asset is under the root restriction
	*
	* @param object	 &$asset	The asset to check
	*
	* @return TRUE
	* @access public
	*/
	function checkRoot(&$asset)
	{
		// This is the root node, will allow it
		$root_nodes = $this->getRootNodes();
		if (in_array($asset->id, $root_nodes)) return TRUE;

		// If a shadow link test the bridge instead
		if (strpos($asset->id, ':') !== FALSE) {
			$matches = explode(':', $asset->id);
			$test_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[0]);
		} else {
			$test_asset = $asset;
		}//end if

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($test_asset->id);
		unset($test_asset);
		// Compare our arrays
		$parent_nodes = array_keys($parents);
		$result = array_intersect($root_nodes, $parent_nodes);

		// Check to see if we have matching array keys
		if (!empty($result)) {
			return TRUE;
		} else {
			return FALSE;
		}//end if

	}//end checkRoot


	/**
	 * Return a list of the root nodes
	 *
	 * @return array
	 * @access public
	 */
	public function getRootNodes()
	{
		$root_nodes   = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', TRUE, 'major', 'root');
		$nodes = Array();
		foreach ($root_nodes as $node) {
			$node_id = array_get_index($node, 'minorid', 0);
			if (!empty($node_id)) $nodes[] = $node_id;
		}//end foreach

		return $nodes;

	}//end getRootNodes()


	/**
	* updates the lookup of the assetid passed and all its childrens, (runs freestyle HIPO)
	*
	* @return assetid
	* @access public
	*/
	public function updateLookupsforAsset($assetid)
	{
		require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/hipo_job_update_lookups.inc';
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$vars = Array('assetids' => Array($assetid));
		$hh->freestyleHipo('hipo_job_update_lookups', $vars);

	}//end updateLookupsforAsset()


	/**
	* Description: get the asset/global/simple_edit level keyword replacement for the passed in asset
	*
	* @param object	 $asset	The asset to fetch replacements for
	* @param array	 $keywords	array of keywords to get replacement for
	*
	* @return array
	* @access public
	*/
	public function getKeywordsReplacements($asset, $keywords)
	{
		$return_array = Array();
		$se_level_keywords = Array();
		//lets split asset/global level keywords as opposed to simple edit layout keywords
		foreach ($keywords as $index => $keyword) {
			if(!(strpos($keyword, '%asset_') === 0 || strpos($keyword, '%globals_') === 0) ) {
				//seems like its a simple edit screen keywords
				$se_level_keywords[] = $keyword;
				unset($keywords[$index]);
			}
		}

		foreach ($keywords as $keyword) {
			$trimmed_keyword = trim($keyword,'%');
			if (strpos($keyword, '%globals_') === 0) {
				$return_array[$trimmed_keyword] = replace_global_keywords($keyword);
			} else if (strpos($keyword, '%asset_') === 0 ) {
				$return_array[$trimmed_keyword] = $asset->getKeywordReplacement($trimmed_keyword);
			}
			if (empty($return_array[$trimmed_keyword])) $return_array[$trimmed_keyword] = 'Could not get replacement for keyword : '.$trimmed_keyword ;
		}

		if(!empty($se_level_keywords)) {
			// create layout on the fly
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('layout');
			$layout = new Layout();
			// create limbo outputter on fly
			require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
			$limbo_outputter = new Limbo_Outputter();
			// get asset_edit_interface
			require_once SQ_SYSTEM_ROOT.'/core/include/asset_edit_interface.inc';
			$ei = new Asset_Edit_Interface($asset->type());

			foreach ($se_level_keywords as $index => $se_keyword) {
				$se_keyword = trim($se_keyword,  '%');
				$layout->layout_keywords = $layout->getLayoutKeywordsArray(Array ($se_keyword));
				//sort order isnt really important here, so we always use like follows
				$screen_sort_order = Array ('0' => 'custom', '1' => 'screen');
				$return_array[$se_keyword] = $ei->getKeywordsReplacementsForPaint($asset, $limbo_outputter, $layout, $screen_sort_order, FALSE, TRUE);
				// lets replace % with &#37; so that while rendering on frontend %asset_name% (keywords)
				// are not replaced and the raw keyword is spat out. but also make sure to do this only to keywords
				$pattern = '/(.)(%)([asset_|globals_]?[a-zA-Z0-9_:\^]*)(%)(.)/';
				$replacement = '$1&#37;$3&#37;$5';
				$return_array[$se_keyword] = preg_replace($pattern, $replacement ,$return_array[$se_keyword]);
				$pattern_2 = '/([\"|\']{0}%)([asset_|globals_]?[a-zA-Z0-9_:\^]*)(%[\"|\']{0})/';
				$replacement_2 = '&#37;$2&#37;';
				$return_array[$se_keyword] = preg_replace($pattern_2, $replacement_2 ,$return_array[$se_keyword]);

				if (empty($return_array[$se_keyword])) $return_array[$se_keyword] = 'Could not get replacement for keyword : '.$se_keyword ;
			}
		}
		return $return_array;

	}//end getKeywordsReplacements()


	/**
	* Description: trys to acquire locks on the asset
	*
	* @param object	 $asset				the asset to acquire locks for
	* @param string	 $screen			the screen for which to acquire locks for
	* @param boolean $dependants_only	whether dependants only or all children, defaults to true
	* @param boolean $force_acquire		whether to attempt to forceably acquire the lock or not,  defaults to false
	*
	* @return array
	* @access public
	*/
	public function acquireLocks(Asset $asset, $screen, $dependants_only , $force_acquire)
	{
		$return_array = Array();

		//if the we arent trying to get all locks and if the locks name specified doesnt exist for this asset then start complaining
		if(!in_array($screen, $GLOBALS['SQ_SYSTEM']->am->getLockTypes($asset->id, 'all')) && $screen != 'all') {
			$return_array[] = '"'.$screen.'" type lock doesnt exist for Asset "'.$asset->name.'" (#'.$asset->id.').';
			return $return_array;
		}
		
		//lets prepare a hipo running vars
		$res = NULL;
		$locks = $this->getLocksInfo($asset->id, $screen);
		if (empty($locks) || (!empty($locks) && $force_acquire)) {
			$job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
			require_once $job_dir.'/hipo_job_acquire_locks.inc';
			$init_hipo = new Hipo_Job_Acquire_Locks();
			$running_vars = Array(
								'assetids'          => Array($asset->id),
								'lock_type'         => $screen,
								'forceably_acquire' => $force_acquire,
								'dependants_only'   => $dependants_only,
							);
			$init_hipo->setRunningVars($running_vars);
			$init_hipo->_steps = $init_hipo->getInitialStepData();
			if (!$init_hipo->prepare()) {
				$return_array[] = 'Could not initialise HIPO job';
				return $return_array;
			}
			set_error_handler(Array(&$init_hipo, '_errorHandler'));
				$init_hipo->freestyle();
			restore_error_handler();
			$return_array = $init_hipo->getErrors();
		} else {
			foreach ($locks as $screen_lock => $lock_info) {
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock_info['userid']);
				$expires_in = easy_time_total(($lock_info['expires'] - time()), TRUE);
				if (!$expires_in) $expires_in = '1 second';
				$return_array[] = '"'.$screen_lock.'" locks are held by User "'.$user->name.'" (#'.$lock_info['userid'].') for Asset "'.$asset->name.'" (#'.$asset->id.'). This is due to expire in '.$expires_in;
			}
		}
		return array_unique($return_array);
	
	}//end acquireLocks()


	/**
	* Returns an array of information about the current lock on this asset (if any)
	*
	* @param string		$assetid		the assetid of the asset that we want lock info for
	* @param string		$lock_type		any valid lock type for the passed asset's type, plus 'all' which will return all locks that this asset has - 
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLockInfo()
	*/
	public function getLocksInfo($assetid, $lock_types)
	{
		$return_array =  $GLOBALS['SQ_SYSTEM']->am->getLockInfo($assetid, $lock_types);
		foreach ($return_array as $screen => $lock_info){
			if (empty($lock_info)){
				unset($return_array[$screen]);
			}
		}
		if($lock_types != 'all' && !empty($return_array)) $return_array = Array ($return_array);

		return $return_array;

	}//end getLocksInfo()


	/**
	* sets the asset status that is passed
	*
	* @param string		$assetid				the assetid of the asset that we want lock info for
	* @param int		$status					status we want to set to
	* @param boolean	$cascade				if the status change should be cascaded to non dependant children assets, default to false
	* @param string		$current_status_desc	description of the current status
	* @param string		$to_be_status_desc		description of the status to be changed to
	*
	* @return array
	* @access public
	*/
	public function setAssetStatus(Asset $asset, $status, $cascade, $current_status_desc, $to_be_status_desc, $workflow_stream)
	{

		$return_array = Array();
		//if status is being set to something we are not allowed to set then start complaining
		if(!(in_array($status, array_keys($asset->getAvailableStatii()))) && !($status == $asset->status && $cascade) ) {
			$return_array[] = '"'.$to_be_status_desc.'" is not a valid option for the "'.$asset->name.'" asset (#'.$asset->id.') in its current "'.$current_status_desc.'" state';
		return $return_array;
		}

		//lets prepare a hipo running vars
		$job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
		require_once $job_dir.'/hipo_job_edit_status.inc';
		$init_hipo = new HIPO_Job_Edit_Status();
		$running_vars = Array(
                            'assetid'           => $asset->id,
                            'new_status'        => $status,
                            'dependants_only'   => !$cascade,
                        );
		if ($workflow_stream !== 'SQ_USE_DEFAULT') {
			$running_vars['workflow_stream'] = $workflow_stream;
		}

		$init_hipo->setRunningVars($running_vars);
		$init_hipo->_steps = $init_hipo->getInitialStepData();
		if (!$init_hipo->prepare()) {
			$return_array[] = 'Could not initialise HIPO job';
			return $return_array;
		}
		set_error_handler(Array(&$init_hipo, '_errorHandler'));
			$init_hipo->freestyle();
		restore_error_handler();
		$return_array = $init_hipo->getErrors();

		return $return_array;

	}//end setAssetStatus()


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
			$this->returnJSON($data);
		}//end if

	}//end returnError()


	/**
	* Returns the JSON output 
	*
	* @param array	 $data	The array of information to return as JSON	
	*
	* @access public
	*/
	function returnJSON($data, $type='')
	{
		// Are we allowed to send JSON? OR return JSON when the function is a 'get' OR on error which will be empty type
		if ($this->attr('allow_json') || (!empty($type) && (strpos($type, 'get') === 0))) {
			// Send our data as JSON
			if (!empty($data)) {
				if (!function_exists('json_encode')) {
					require_once 'Services/JSON.php';
					$json = new Services_JSON();
					$output = $json->encode($data);
				} else {
					$output = json_encode($data);
				}//end else

				// Send our JSON
				$output = preg_replace('/(%){1}([globals_snippet_][_A-Za-z0-9\-]*)(%){1}/','&#37;$2&#37;', $output);
				echo $output;

			}//end if

		}//end if

	}//end returnJSON()


	/**
	* getRandomFileName get a random filename for the file being uploaded
	*
	* @param string $file_name  The original name of the file
	*
	* @return string
	* @access public
	*/
	function getRandomFilename($file_name)
	{
		$hash_file_name     = hash('md5', $file_name.time());
		$rand_position      = rand(0, strlen($hash_file_name)-6);
		$tmp_file_name      = substr($hash_file_name, $rand_position, 6);

		return $tmp_file_name;

	}//end getRandomFileName()


	/*
	* Purpose : provides an extension for filename for the provided type_code
	*
	* @param string	type_code	file type_code we are fishing for
	*
	* @return string
	* @access public
	*/
	function get_file_extension($type_code)
	{
		$ext_array = Array(
						'file'				=> 'txt',
						'powerpoint_doc'	=> 'ppt',
						'xml_file'			=> 'xml',
						'flv_file'			=> 'flv',
						'image'				=> 'jpg',
						'css_file'			=> 'css',
						'video_file'		=> 'wmv',
						'mp3_file'			=> 'mp3',
						'text_file'			=> 'txt',
						'rtf_file'			=> 'rtf',
						'excel_doc'			=> 'xls',
						'pdf_file'			=> 'pdf',
						'xsl_file'			=> 'xsl',
						'js_file'			=> 'js',
						'word_doc'			=> 'doc',
						'data_source_graph'	=> 'grf',
						'thumbnail'			=> 'jpg',
					 );

		return $ext_array[$type_code];

	}//end get_file_extension()


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
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
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
			}//end catch
		}//end catch
	}//end catch

	if (!request) {
		alert('Your browser does not support Ajax');
	}

	return request;

}//end createRequest


/**
* Checks to see if a variable is set
*
* @param string		obj		The variable we check
*
* @version $Revision: 0.1
*/
function isset(obj)
{
	// Check to see if a variable or array item is set
	if (typeof(obj) == 'undefined' || obj == null) {
		return false;
	} else {
		return true;
	}

}//end isset


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

}//end jsonToObj
	

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
			}//end if
			
		}//end if

	}//end if

}//end success


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

}//end setApiKey


/**
* Make our ajax request
*
* @param string		url				The url to send to the server
* @param boolean	receive			Should we even use a callback
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function makeRequest(url, receive, dataCallback)
{
	//split url to url and parameters
	urlarray = url.split("?");
	// Create an instance of our ajax object
	var ajaxRequest = createRequest();
	// Open request
	ajaxRequest.open('POST', encodeURI(urlarray[0]), true);
	// Should we use a callback?
	if (receive) {
		// Custom callback
		ajaxRequest.onreadystatechange = function() {
			success(ajaxRequest, dataCallback);
		};
	}//end if
	ajaxRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	ajaxRequest.send(encodeURI(urlarray[1]));

}//end makeRequest


/**
* Recursive helper function to write out all properties of an object
*
* @param object		obj				The JSON object
* @param object		parent			Parent JSON object
*
* @version $Revision: 0.2
*/
function dumpObj(obj, parent) {
	// Go through all the properties of the passed-in object
	for (var i in obj) {
		if (parent) {
			var msg = parent + '.' + i + ' => ' + obj[i] + '<br />';
		} else {
			var msg = i + " => " + obj[i] + "<br>";
		}
		// Write it out
		document.write(msg);
		// Check if we need to go deeper
		if (typeof obj[i] == 'object') {
			// Write opening
			document.write('<div style="padding-left:20px;">');
			if (parent) {
				dumpObj(obj[i], parent + '.' + i);
			} else {
				dumpObj(obj[i], i);
			}
			// Write closing
			document.write('</div>');
		}//end if

	}//end for

}//end dumpObj
  

/**
* This will return general information about the asset
*
* @param string		asset_id		Id of the asset we are getting info for
* @param boolean	get_attributes	if we are getting non standard attribute values of the assets(FALSE by default)
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function getGeneral(asset_id, get_attributes, dataCallback) 
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var get_attributes = isset(get_attributes) ? get_attributes : 0;

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getGeneral&id=' + asset_id + '&get_attributes=' + get_attributes;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getGeneral
	

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

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getAttributes


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
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setAttribute&id=' + asset_id + '&attr_name=' + attr_name + '&attr_val=' + attr_val.replace(/#/g , "%23").replace(/&/g , "%26").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setAttribute


/**
* This will set an multiple attributes value for an Asset
*
* @param string		asset_id		Id of the asset we are getting info for
* @param array		field_info		Attribute name and their respective values to be changed to
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function setMultipleAttributes(asset_id, field_info, dataCallback) 
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var field_names = '';
	var field_vals = '';
	for (var field_name in field_info) {
		// construct our query strings to be passed
		if (field_name == '') continue;
		field_names = field_names + field_name + '\\,';
		field_vals = field_vals + field_info[field_name].replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,' ;
	}

	// remove the trailing "\,"
	field_names = field_names.substring(0, field_names.length-2);
	field_vals = field_vals.substring(0,field_vals.length-2);

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setMultipleAttributes&id=' + asset_id + '&attr_name=' + field_names + '&attr_val=' + field_vals;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setMultipleAttributes


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

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getMetadata


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
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setMetadata&id=' + asset_id + '&field_id=' + field_id + '&field_val=' + field_val.replace(/#/g , "%23").replace(/&/g , "%26").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setMetadata


/**
* Set metadata values of multiple fields for an asset
*
* @param string		asset_id		Id of asset to set metadata for
* @param array 		field_info		Field Ids and their values
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function setMetadataAllFields(asset_id, field_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var field_ids = '';
	var field_vals = '';
	for (var field_id in field_info) {
		// construct our query strings to be passed
		field_ids = field_ids + field_id + '\\,';
		field_vals = field_vals + field_info[field_id].replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,' ;
	}

	// remove the trailing "\,"
	field_ids = field_ids.substring(0, field_ids.length-2);
	field_vals = field_vals.substring(0,field_vals.length-2);

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setMetadata&id=' + asset_id + '&field_id=' + field_ids + '&field_val=' + field_vals;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setMetadataAllFields()


/**
* This will send an asset to the trash
*
* @param array | string		asset_ids		Id of the asset(s) to delete
* @param function			dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function trashAsset(asset_ids, dataCallback)
{
	if (typeof(asset_ids) != 'string') {
		// Create blank function
		var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
		var ids = '';

		for (var index in asset_ids) {
			// construct our query strings to be passed
			ids = ids + asset_ids[index] + '\\,';
		}

		// remove the trailing "\,"
		ids = ids.substring(0, ids.length-2);
	} else {
		ids = asset_ids;
	}

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=trashAsset&assetid=' + ids;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end trashAsset


/**
* This will return child asset ids of the passed asset
*
* @param string		asset_id		Id of the asset to get children of
* @param number		levels			Number of levels to return
* @param array		type_codes		asset type_code that we want back
* @param array		link_type		type of links we are looking for
*									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param array		link_values		link values allowed on the asset returned
* @param boolean	get_attributes	if we are getting non standard attribute values of the assets(FALSE by default)
* @param function	dataCallback	Custom callback function
*
*
* NOTE : type_code, link_type and link_values are empty by default. So matrix will return everything
*		 also these options work for direct links only i.e. levels = 1
*
* @version $Revision: 0.2
*
*/
function getChildren(asset_id, levels, type_codes, link_types, link_values, get_attributes, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var type_code = '';
	if(isset(type_codes) && type_codes != '') {
		if(typeof(type_codes) == 'object' ) {
			for (var idx in type_codes) {
				type_code = type_code + type_codes[idx] + '\\,';
			}
			type_code = type_code.substring(0, type_code.length-2);
		} else {
			type_code = type_codes;
		}
	}

	var link_type = '';
	if(isset(link_types) && link_types != '') {
		if(typeof(link_types) == 'object' ) {
			for (var idx in link_types) {
				link_type = link_type + link_types[idx] + '\\,';
			}
			link_type = link_type.substring(0, link_type.length-2)
		} else {
			link_type = link_types;
		}
	}

	var link_value = '';
	if(isset(link_values) && link_values != '') {
		if(typeof(link_values) == 'object' ) {
			for (var idx in link_values) {
				link_value = link_value + link_values[idx] + '\\,';
			}
			link_value = link_value.substring(0, link_value.length-2);
		} else {
			link_value = link_values;
		}
	}

	// Check to see if we have set any levels
	var levels = typeof(levels) != 'undefined' ? levels : 0;
	var get_attributes = isset(get_attributes) ? get_attributes : 0;

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' +
				'?key=' + api_key +
				'&type=getChildren&id=' + asset_id +
				'&depth=' + levels +
				'&type_code=' + type_code +
				'&link_type=' + link_type +
				'&link_value=' + link_value +
				'&get_attributes=' + get_attributes;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getChildren


/**
* This return parents of the passed id
*
* @param string		asset_id		Id of the asset to get parents of
* @param number		levels			Number of levels to return
* @param array		type_codes		asset type_code that we want back
* @param array		link_type		type of links we are looking for
*									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param array		link_values		link values allowed on the asset returned
* @param boolean	get_attributes	if we are getting non standard attribute values of the assets(FALSE by default)
* @param function	dataCallback	Custom callback function
*
*
* NOTE : type_code, link_type and link_values are empty by default. So matrix will return everything
*		 also these options work for direct links only i.e. levels = 1
*
* @version $Revision: 0.2
*
*/
function getParents(asset_id, levels, type_codes, link_types, link_values, get_attributes, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var type_code = '';
	if(isset(type_codes) && type_codes != '') {
		if(typeof(type_codes) == 'object' ) {
			for (var idx in type_codes) {
				type_code = type_code + type_codes[idx] + '\\,';
			}
			type_code = type_code.substring(0, type_code.length-2);
		} else {
			type_code = type_codes;
		}
	}

	var link_type = '';
	if(isset(link_types) && link_types != '') {
		if(typeof(link_types) == 'object' ) {
			for (var idx in link_types) {
				link_type = link_type + link_types[idx] + '\\,';
			}
			link_type = link_type.substring(0, link_type.length-2);
		} else {
			link_type = link_types;
		}
	}

	var link_value = '';
	if(isset(link_values) && link_values != '') {
		if(typeof(link_values) == 'object' ) {
			for (var idx in link_values) {
				link_value = link_value + link_values[idx] + '\\,';
			}
			link_value = link_value.substring(0, link_value.length-2);
		} else {
			link_value = link_values;
		}
	}

	// Check to see if we have set any levels
	var levels = typeof(levels) != 'undefined' ? levels : 0;
	var get_attributes = isset(get_attributes) ? get_attributes : 0;

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key +
				'&type=getParents&id=' + asset_id +
				'&depth=' + levels +
				'&type_code=' + type_code +
				'&link_type=' + link_type +
				'&link_value=' + link_value +
				'&get_attributes=' + get_attributes;


	// Make our request
	makeRequest(url, true, dataCallback);

}//end getParents

/**
* This returns permissions for an asset
*
* @param string		asset_id		Id of the asset to get permissions for
* @param string		level			1=READ 2=WRITE 3=ADMIN
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getPermissions(asset_id, level, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getPermissions&id=' + asset_id + '&level=' + level;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getPermissions


/**
* Creates an asset
*
* @param integer	parent_id			Parentid of the new parent
* @param string		type_code			Type code of new asset
* @param string		asset_name			Name for new asset
* @param integer	link_type			Type of link to create
* @param string		link_value			Value of the link
* @param integer	sort_order			Order in the tree
* @param integer	is_dependant		Dependant to parent
* @param integer	is_exclusive		Exclusive to parent
* @param integer	extra_attributes	Allows additional attributes
* @param string		attributes			String of additional query string containing key/pair values
*
* @version $Revision: 0.2
*/
function createAsset(parent_id, type_code, asset_name, link_type, link_value, sort_order, is_dependant, is_exclusive, extra_attributes, attributes, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	if (!isset(link_type)) link_type = '';
	if (!isset(link_value)) link_value = '';
	if (!isset(sort_order)) sort_order = '';
	if (!isset(is_dependant)) is_dependant = '';
	if (!isset(is_exclusive)) is_exclusive = '';
	if (!isset(extra_attributes)) extra_attributes = '';
	if (!isset(attributes)) attributes = '';

	// Build our string
	var url =	'<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=createAsset' + 
				'&id=' + parent_id +
				'&type_code=' + type_code +
				'&asset_name=' + asset_name.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +
				'&link_type=' + link_type +
				'&link_value=' + link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +
				'&sort_order=' + sort_order +
				'&is_dependant=' + is_dependant +
				'&is_exclusive=' + is_exclusive +
				'&extra_attributes=' + extra_attributes +
				'&' + attributes;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end createAsset


/**
* Returns asset type codes
*
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getAssetTypes(dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getAssetTypes&id=1';

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getAssetTypes


/**
* Gets information for lock type passed
*
* @param string		asset_id		Id of the asset to get locks for
* @param string		screen_name		The screen to get locks for
* @param function	dataCallback	Custom callback function
*
*
*/
function getLocksInfo(asset_id, screen_name, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// If the user does not set it, we get all locks
	if (!isset(screen_name) || screen_name == '') screen_name = 'all';

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getLocksInfo&id=' + asset_id + '&screen=' + screen_name;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getLocksInfo()


/**
* Acquires a lock
*
* @param string		asset_id		Id of the asset to get locks for
* @param string		screen_name		The screen to get locks for
* @param boolean 	dependants_only	whether dependants only or all children, defaults to true
* @param boolean 	force_acquire	whether to attempt to forceably acquire the lock or not,  defaults to false
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function acquireLock(asset_id, screen_name, dependants_only, force_acquire, dataCallback)
{
	var dependants_only = isset(dependants_only) && ( dependants_only.toString().toLowerCase() != 'null' && dependants_only != '' ) ? dependants_only : 1;
	var force_acquire = isset(force_acquire) && ( force_acquire.toString().toLowerCase() != 'null' && force_acquire != '' ) ? force_acquire : 0;

	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	// but try conversion only if its a string still
	if (typeof(dependants_only) == 'string') {
		if (dependants_only.toLowerCase() == 'false') {
			dependants_only = 0;
		} else if (dependants_only.toLowerCase() == 'true') {
			dependants_only = 1;
		}
	}

	if (typeof(force_acquire) == 'string') {
		if (force_acquire.toLowerCase() == 'false') {
			force_acquire = 0;
		} else if (force_acquire.toLowerCase() == 'true') {
			force_acquire = 1;
		}
	}

	// If the user does not set it, we get all locks
	if (!isset(screen_name) || screen_name == '') screen_name = 'all';

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=acquireLock&id=' + asset_id + '&screen=' + screen_name + '&dependants_only=' + dependants_only + '&force_acquire=' + force_acquire;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end acquireLock


/**
* Releases a lock
*
* @param string		asset_id		Id of the asset to release locks for
* @param string		screen_name		The screen to release locks for
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function releaseLock(asset_id, screen_name, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// If the user does not set it, we get all locks
	if (!isset(screen_name) || screen_name == '') screen_name = 'all';

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=releaseLock&id=' + asset_id + '&screen=' + screen_name;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end releaseLock


/**
* Creates a link between two assets
*
* @param integer	parent_id		Major asset id we are linking from
* @param integer	child_id		Minor asset id we are linking to
* @param integer	link_type		Type of link to create
* @param string		link_value		Value of the link
* @param integer	sort_order		Order in the tree
* @param integer	is_dependant	Dependant to parent
* @param integer	is_exclusive	Exclusive to parent
*
* @version $Revision: 0.2
*/
function createLink(parent_id, child_id, link_type, link_value, sort_order, is_dependant, is_exclusive, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	if (!isset(link_type)) link_type = '1';
	if (!isset(link_value)) link_value = '';
	if (!isset(sort_order)) sort_order = '';
	if (!isset(is_dependant)) is_dependant = '';
	if (!isset(is_exclusive)) is_exclusive = '';

	// Build our string
	var url =	'<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=createLink' + 
				'&id=' + child_id +
				'&parent_id=' + parent_id +
				'&link_type=' + link_type +
				'&link_value=' + link_value +
				'&sort_order=' + sort_order +
				'&is_dependant=' + is_dependant +
				'&is_exclusive=' + is_exclusive;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end createLink


/**
* Removes a link between a parent and child
*
* @param string		parent_id		Id of the parent
* @param string		child_id		Id of the child
* @param string     link_type       type of link we are looking for (SQ_LINK_TYPE_1 link by default)
									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param string     link_value      value of link we are looking for ('' by default)
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function removeLink(parent_id, child_id, link_type, link_value, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var link_type = (isset(link_type) && link_type != '' ) ? link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	var link_value = isset(link_value) ? link_value : '';

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=removeLink&id=' + child_id + '&parent_id=' + parent_id + '&link_type=' + link_type + '&link_value=' + link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end removeLink


/**
* Removes multiple links between parent and child pairs
*
* @param json object	link_info		array of link_info
* example :
* 		var link_info = {
* 			"links":[
* 				{
* 					"parent": parent,
* 					"child": child,
* 					"link_type": link_type,
*					"link_value": link_value,
* 				},
* 				{
* 					"parent": parent_2,
* 					"child": child_2,
* 					"link_type": link_type_2,
*					"link_value": link_value,
* 				},
* 				{
* 					"parent": parent_3,
* 					"child": child_3,
* 					"link_type": link_type_3,
*					"link_value": link_value,
* 				},
* 			]
* 		 };
*
* values in JSON object
* 		string   	  	parent			Id of the parent
* 		string    		child			Id of the child
* 		string    		link_type		type of link we are looking for(SQ_LINK_TYPE_1 link by default)
*										Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* 		string 	    	link_value		value of link we are looking for ('' by default)
* @param function		dataCallback	Custom callback function
*
*/
function removeMultipleLinks(link_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var parentid = '';
	var childid = '';
	var link_type = '';
	var link_value = '';

	for (var x = 0; x < link_info.links.length; x++) {
		link_typ = isset(link_info.links[x].link_type) ?  link_info.links[x].link_type.toUpperCase() : 'SQ_LINK_TYPE_1' ;
		link_val = isset(link_info.links[x].link_value) ?  link_info.links[x].link_value : '' ;

		parentid = parentid + link_info.links[x].parent + '\\,';
		childid = childid + link_info.links[x].child + '\\,';
		link_type = link_type + link_typ + '\\,';
		link_value = link_value + link_val.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,';
	}

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=removeMultipleLinks&parent_id=' + parentid.substring(0, parentid.length-2) + 
				'&child_id=' + childid.substring(0, childid.length-2) +
				'&link_type=' + link_type.substring(0, link_type.length-2) +
				'&link_value=' + link_value.substring(0, link_value.length-2)

	// Make our request
	makeRequest(url, true, dataCallback);

}//end removeMultipleLinks


/**
* Moves a link from one parent to another
*
* @param string		old_parent_id			Id of the old parent
* @param string		child_id				Id of the child
* @param string		old_link_type			Type of link we are searching for between given assets (SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3
* @param string		old_link_value			Value of link we are searching for between given assets ('' by default)
* @param string		new_parent_id			Id of the new parent
* @param string		new_link_type			Type of link to use (SQ_LINK_TYPE_1 link by default)
* 											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3
* @param string		new_link_value			Value of link to use ('' by default)
* @param string		new_position			The new position
* @param function	dataCallback			Custom callback function
*
* @version $Revision: 0.2
*/
function moveLink(old_parent_id, child_id, old_link_type, old_link_value, new_parent_id, new_link_type, new_link_value, new_position, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	var old_link_type = (isset(old_link_type) && old_link_type != '' ) ? old_link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	var old_link_value = isset(old_link_value) ? old_link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") : '';

	var new_link_type = (isset(new_link_type) && new_link_type != '' ) ? new_link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	var new_link_value = isset(new_link_value) ? new_link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") : '';
	new_position      = (isset(new_position) && new_position != '') ? new_position : '0';

	// Build our string
	var url =	'<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=moveLink' + 
				'&id=' + child_id +
				'&old_parent_id=' + old_parent_id  +
				'&old_link_type=' + old_link_type  +
				'&old_link_value='+ old_link_value +
				'&new_parent_id=' + new_parent_id  +
				'&new_link_type=' + new_link_type  +
				'&new_link_value='+ new_link_value +
				'&new_position='  + new_position;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end moveLink


/**
* Updates a link
*
* @param string		parent_id				id of the parent
* @param string		child_id				id of the child
* @param string		existing_link_type		existing link type betweent the assets (SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param string		existing_link_value		existing link value betweent the assets ('' by default)
* @param string		link_type				link type to be updated to (SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param string		link_value				link value to be updated to ('' by default)
* @param string		new_position			the new position
* @param string		locked					the asset link lock status(locked by default)
* @param function	dataCallback			custom callback function
*
* @version $Revision: 0.2
*/
function updateLink(parent_id, child_id, existing_link_type, existing_link_value, link_type, link_value, sort_order, locked, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	existing_link_type = (isset(existing_link_type) && existing_link_type != '' ) ? existing_link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	existing_link_value= isset(existing_link_value) ? existing_link_value : '';

	link_type = (isset(link_type) && link_type != '' ) ? link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	link_value= isset(link_value) ? link_value : '';

	sort_order= isset(sort_order) ? sort_order : '';
	locked    = isset(locked) ? locked : '';

	// Build our string
	var url =	'<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=updateLink' + 
				'&id=' 					+ child_id +
				'&parent_id=' 			+ parent_id +
				'&existing_link_type='	+ existing_link_type +
				'&existing_link_value='	+ existing_link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +
				'&link_type=' 			+ link_type +
				'&link_value=' 			+ link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +
				'&sort_order=' 			+ sort_order +
				'&locked=' 				+ locked;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end updateLink


/**
* Updates multiple links
*
* @param json object	link_info		Array of all the link information
* example :
* 		var link_info = {
* 			"links":[
* 				{
* 					"parent": parent,
* 					"child": child,
*					"existing_link_type": ext_link_type,
*					"existing_link_value": ext_link_value,
* 					"link_type": link_type,
* 					"link_value": link_value,
* 					"sort_order": sort_order,
* 					"link_lock": link_lock,
* 				},
* 				{
* 					"parent": parent_2,
* 					"child": child_2,
*					"existing_link_type": ext_link_type_2,
*					"existing_link_value": ext_link_value_2,
* 					"link_type": link_type_2,
* 					"link_value": link_value_2,
* 					"sort_order": sort_order_2,
* 					"link_lock": link_lock_2,
* 				},
* 				{
* 					"parent": parent_3,
* 					"child": child_3,
*					"existing_link_type": ext_link_type_3,
*					"existing_link_value": ext_link_value_3,
* 					"link_type": link_type_3,
* 					"link_value": link_value_3,
* 					"sort_order": sort_order_3,
* 					"link_lock": link_lock_3,
* 				},
* 			]
* 		 };
*
* values in JSON object
*       string          parent				Id of the parent
*       string          child				Id of the child
*       string          existing_link_type	type of link we are looking for(SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
*       string          existing_link_value	value of link we are looking for ('' by default)
*       string          link_type			type of link we changing to (SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
*       string          link_value			value of link we are changing it to ('' by default)
*		string			sort_order			sort order we want to update the link to
*		boolean			link_lock			link lock value we want to set it to(default to TRUE)
*
* @param function		dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function updateMultipleLinks(link_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var parentid = '';
	var childid = '';
	var existing_link_type = '';
	var existing_link_value= '';
	var link_type = '';
	var link_value = '';
	var sort_order = '';
	var locked = '';

	for (var x = 0; x < link_info.links.length; x++) {
		existing_link_typ	= (isset(link_info.links[x].existing_link_type) && link_info.links[x].existing_link_type != '' ) ?  link_info.links[x].existing_link_type.toUpperCase() : 'SQ_LINK_TYPE_1' ;
		existing_link_val	= isset(link_info.links[x].existing_link_value) ?  link_info.links[x].existing_link_value : '' ;
		link_typ			= isset(link_info.links[x].link_type) ?  link_info.links[x].link_type.toUpperCase() : 'SQ_LINK_TYPE_1' ;
		link_val			= isset(link_info.links[x].link_value) ?  link_info.links[x].link_value : '' ;
		sort_ord			= isset(link_info.links[x].sort_order) ?  link_info.links[x].sort_order : '' ;
		lock				= isset(link_info.links[x].link_lock) ?  link_info.links[x].link_lock : '' ;

		parentid			= parentid + link_info.links[x].parent + '\\,';
		childid				= childid + link_info.links[x].child + '\\,';
		existing_link_type	= existing_link_type + existing_link_typ +'\\,';
		existing_link_value = existing_link_value + existing_link_val.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +'\\,';
		link_type			= link_type + link_typ + '\\,';
		link_value			= link_value + link_val.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,';
		sort_order			= sort_order + sort_ord + '\\,';
		locked				= locked + lock + '\\,';
	}

	// Build our string
	var url =	'<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=updateMultipleLinks' + 
				'&parent_id=' + parentid.substring(0, parentid.length-2) +
				'&child_id=' + childid.substring(0, childid.length-2) +
				'&existing_link_type=' + existing_link_type.substring(0, existing_link_type.length-2) +
				'&existing_link_value=' + existing_link_value.substring(0, existing_link_value.length-2) +
				'&link_type=' + link_type.substring(0, link_type.length-2) +
				'&link_value=' + link_value.substring(0, link_value.length-2) +
				'&sort_order=' + sort_order.substring(0, sort_order.length-2) +
				'&locked=' + locked.substring(0, locked.length-2);

	// Make our request
	makeRequest(url, true, dataCallback);

}//end updateMultipleLinks()



/**
* Returns the link id between a parent and child
*
* @param string		parent_id		Id of the parent
* @param string		child_id		Id of the child
* @param string		link_type		type of link we are looking for(SQ_LINK_TYPE_1 link by default)
									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param string		link_value		value of link we are looking for ('' by default)
* @param boolean	all_info		if we want all the link information or just linkid
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getLinkId(parent_id, child_id, link_type, link_value, all_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var all_info = isset(all_info) ? all_info : 0;
	var link_type = (isset(link_type) && link_type != '' ) ? link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	var link_value = isset(link_value) ? link_value	: '';

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getLinkId&id=' + child_id + '&parent_id=' + parent_id + '&all_info=' + all_info + '&link_type=' + link_type + '&link_value=' + link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getLinkId


/**
* This will return tree information for children
*
* @param string		asset_id		Id of the asset to get children of
* @param number		levels			Number of levels to return
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getAssetTree(asset_id, levels, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set any levels
	var levels = typeof(levels) != 'undefined' ? levels : 0;

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getAssetTree&id=' + asset_id + '&depth=' + levels;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getAssetTree


/**
* This will get replacements for the passed in keywords
*
* @param string		asset_id		Id of the asset to get children of
* @param array		keywords		Array of keywords to get replacements for
* @param function	dataCallback	Custom callback function
*
*/
function getKeywordsReplacements(asset_id, keywords_array, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var keywords = '';
	for (var keyword in keywords_array) {
		// construct our query strings to be passed
		if (keywords_array[keyword] == '') continue;
		keywords = keywords + keywords_array[keyword] + '\\,';
	}

	// remove the trailing "\,"
	keywords = keywords.substring(0, keywords.length-2);

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getKeywordsReplacements&id=' + asset_id + '&keywords=' + keywords;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getKeywordsReplacements()


/**
* This will set Asset to the status that is passed in
*
* @param string		asset_id		id of the asset to get children of
* @param int		status			status tha asset is to be set to
* @param boolean	cascade			if to cascade the status to non-dependant children(false by default)
* @param string		workflow_stream workflow stream to be passed in
* @param function	dataCallback	custom callback function
*
*
*/
function setAssetStatus(asset_id, status, cascade, workflow_stream, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var workflow_stream  = isset(workflow_stream) ? workflow_stream : '';
	var cascade = isset(cascade) && ( cascade.toString().toLowerCase() != 'null' && cascade != '' ) ?  cascade : 0;

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	// but try conversion only if its a string still
	if (typeof(cascade) == 'string') {
		if (cascade.toLowerCase() == 'false') {
			cascade = 0;
		} else if (cascade.toLowerCase() == 'true') {
			cascade = 1;
		}
	}

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setAssetStatus&id=' + asset_id + '&status=' + status + '&cascade=' + cascade + '&workflow_stream=' + workflow_stream.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setAssetStatus()


/**
* This will get the Child count for the passed in asset
*
* @param string		asset_id		Id of the asset to get children of
* @param number		level			Number of levels to return, default all
* @param function	dataCallback	Custom callback function
*
*
*/
function getChildCount(asset_id, level, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	// Check to see if we have set any levels
	var level = isset(level) ? level : 0;

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getChildCount&asset_id=' + asset_id + '&depth=' + level;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getChildCount()


/**
* This will get webpath(s) of the assetid supplied
*
* @param string		asset_id		Id of the asset to get webpaths for
* @param function	dataCallback	Custom callback function
*
*
*/
function getWebPath(asset_id, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getWebPath&id=' + asset_id;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getWebPath()


/**
* This will set webpath(s) of the assetid supplied
*
* @param string		asset_id		Id of the asset to get webpaths for
* @param array		paths			new web paths to be assigned to asset
* @param boolean	auto_remap		if we auto remaping(default to TRUE)
* @param function	dataCallback	Custom callback function
*
*
*/
function setWebPath(asset_id, paths, auto_remap ,dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var auto_remap = isset(auto_remap) && ( auto_remap.toString().toLowerCase() != 'null' && auto_remap != '' ) ?  auto_remap : 1;

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	// but try conversion only if its a string still
	if (typeof(auto_remap) == 'string') {
		if (auto_remap.toLowerCase() == 'false') {
			auto_remap = 0;
		} else if (auto_remap.toLowerCase() == 'true') {
			auto_remap = 1;
		}
	}

	var webpath = '';
	for (var path in paths) {
		// construct our query strings to be passed
		webpath = webpath + paths[path].replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,';
	}

	// remove the trailing "\,"
	webpath = webpath.substring(0, webpath.length-2);


	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setWebPath&id=' + asset_id + '&webpath=' + webpath + '&auto_remap=' + auto_remap ;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getWebPath()


/**
* This function will tell us if we have a runnig workflow on the passed assetid
*
* @param string		asset_id		Id of the asset to get webpaths for
* @param boolean	granted			what is the status of workflow we are trying to get (default to NULL)
*									TRUE => granted, FALSE => denied, NULLL => get all
* @param boolean	running			if we only are getting wokflows that are running (default to FALSE)
* @param function	dataCallback	Custom callback function
*
*
*/
function getWorkflowSchema(asset_id, granted, running, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var granted = isset(granted) && ( granted.toString().toLowerCase() != 'null' && granted != '' ) ? granted : '';
	var running = isset(running) && ( running.toString().toLowerCase() != 'null' && running != '' ) ? running : 0;

	// we cannot pass TRUE or FALSE as string so lets do our conversion 
	// but try conversion only if its a string still
	// but try conversion only if its a string still
	if (typeof(granted) == 'string') {
		if (granted.toLowerCase() == 'false') {
			granted = 0;
		} else if (granted.toLowerCase() == 'true') {
			granted = 1;
		}
	}
	if (typeof(running) == 'string') {
		if (running.toLowerCase() == 'false') {
			running = 0;
		} else if (running.toLowerCase() == 'true') {
			running = 1;
		}
	}

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getWorkflowSchema&id=' + asset_id + '&granted=' + granted + '&running=' + running;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getWorkflowSchema()


/**
* This function will create file type asset of the type_code provided
*
* @param	string		parentID		asset ID of parent where the new asset will be linked to
* @param	string		type_code		type_code of the new asset(default to 'file')
* @param	string		friendly_name	friendly name of the new asset to be created
* @param	string		link_type		type of link to create to the parent for this new asset(SQ_LINK_TYPE_1 link by default)
*										Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param	string		link_value		value of link to create to the parent for this new asset('' by default)
* @param	function 	dataCallback	Custom callback function
*
* 
*/
function createFileAsset(parentID, type_code, friendly_name, link_type, link_value, dataCallback)
{
    // Create blank function
    var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
    var type_code = (isset(type_code) && type_code != '' ) ? type_code : 'file';
    var friendly_name  = isset(friendly_name) ? friendly_name : '';
	var link_type = (isset(link_type) && link_type != '') ? link_type : SQ_LINK_TYPE_1;
	var link_value = isset(link_value) ? link_value : '';

    var url = '<?php echo $this->getURL(); ?>' +
                '?key=' + api_key +
				'&type=createFileAsset&id=' + parentID +
				'&type_code=' + type_code +
				'&friendly_name=' + friendly_name +
				'&link_type=' + link_type +
				'&link_value=' + link_value;

    // Make our request
    makeRequest(url, true, dataCallback);


}//end createFileAsset()


/**
* This function will let user edit content of Editable File type assets
* File type that can be edited - css_file, xml_file, css_file, text_file, xsl_file, js_file
* User needs to acquire locks before being able to edit the file
*
*
* @param	string		assetID 		Id of the asset to update content for
* @param	string		content			new content of the asset
* @param	function	dataCallback	Custom callback function
*
*/
function setContentOfEditableFileAsset(assetID, content, dataCallback)
{
	// Create blank function
    var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var content = isset(content) ? escape(content) : 'no_value_provided';

    var url = '<?php echo $this->getURL(); ?>' +
                '?key=' + api_key +
				'&type=setContentOfEditableFileAsset&id=' + assetID +
				'&content=' + content ;

    // Make our request
    makeRequest(url, true, dataCallback);


}// end setContentOfEditableFileAsset()


/**
* This function will let user import assets into Matrix from a structured XML file
* For example for xml file look under System Tools > Import Assets from XML Tool > example.xml
*
*
* @param	string		assetID 		Asset under which the assets are to be imported under
* @param	string		filePath		path to file on the file system
* @param	function	dataCallback	Custom callback function
*
*/
function importAssetsFromXML(assetID, filePath, dataCallback)
{
	// Create blank function
    var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var file = isset(file) ? content : 'no_file_provided';
    var url = '<?php echo $this->getURL(); ?>' +
                '?key=' + api_key +
				'&type=importAssetsFromXML&id=' + assetID +
				'&filePath=' + filePath;

    // Make our request
    makeRequest(url, true, dataCallback);

}// end importAssetsFromXML()


/**
* Returns an array of roles and the users/groups which can perform it
*
* @param int		assetid				the assetid of the asset the role is applied to
* @param int		roleid				the assetid of the the role that is applied
* @param int		userid				the assetid of the user performing the role
* @param boolean	include_assetid		whether to include the assetid in the returned array
* @param boolean	include_globals		whether to query the role view which includes
*										expanded global roles as individual users
* @param boolean	expand_groups		when TRUE, any groups defined within a role will be
*										replaced with the userids in that group.
*										If FALSE, return the groupids
* @param boolean	inc_dependants		If false it will filter out the dependant assets
* @param function	dataCallback		Custom callback function
*
* @return array
* @access public
*/
function getRoles(assetid, roleid, userid, include_assetid, include_globals, expand_groups, inc_dependants, dataCallback) 
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var assetid = isset(assetid) ? assetid : '';
	var roleid = isset(roleid) ? roleid : '';
	var userid = isset(userid) ? userid : '';
	var include_assetid = isset(include_assetid) && ( include_assetid.toString().toLowerCase() != 'null' && include_assetid != '' ) ? include_assetid : 0;
	var include_globals = isset(include_globals) && ( include_globals.toString().toLowerCase() != 'null' && include_globals != '' ) ? include_globals : 0;
	var expand_groups = isset(expand_groups) && ( expand_groups.toString().toLowerCase() != 'null' && expand_groups != '' ) ? expand_groups : 0;
	var inc_dependants = isset(inc_dependants) && ( inc_dependants.toString().toLowerCase() != 'null' && inc_dependants != '' ) ? inc_dependants : 1;

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getRoles&assetid=' + assetid
	 + '&roleid=' + roleid
	 + '&userid=' + userid
	 + '&include_assetid=' + include_assetid
	 + '&include_globals=' + include_globals  
	 + '&expand_groups=' + expand_groups
	 + '&inc_dependants=' + inc_dependants;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getRoles

<?php

	}//end printJs


}//end class


?>
