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
* $Id: js_api.inc,v 1.28 2010/03/22 04:21:47 akarelia Exp $
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
* @version $Revision: 1.28 $
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
			header('Content-Type: text/plain');
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
		$wa = $asset->writeAccess();

		// Check to see what we should return
		switch ($type) {
			case	'getGeneral':
				if ($this->attr('get_general')) {
					if ($ra) {
						$data = $this->getGeneralInfo($asset);
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

							$field_ids = explode('\\,', $_REQUEST['field_id']);
							$field_vals = explode('\\,', $_REQUEST['field_val']);

							foreach($field_vals as $index =>  $fieldVal) {
								$field_vals[$index] = str_replace($replace_these, $replace_with, $fieldVal);
							}

							$this->setAssetMetadata($asset->id, $field_ids, $field_vals);
							$i = 0;
							foreach($field_vals as $index =>  $fieldVal) {
								$data['success'][] = 'Metadata field #'.$field_ids[$i].' has been successfully set to "'.str_replace($replace_these, $replace_with, $fieldVal).'" for Asset "'.$asset->name.'" (#'.$assetid.')';
								$i++;
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

			case	'trashAsset':
				if ($this->attr('trash_asset')) {
					if ($wa) {
						$this->trashAsset($asset->id);
						$data['success'] = 'Asset #'.$asset->id.' is now in the trash';
					} else {
						$this->returnError('You do not have permissions to delete this asset');
					}//end else
				} else {
					$this->returnError('The trashAsset function must be activated');
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
						//$data = $am->getAssetTree($asset->id, $levels);
						$data = $this->getChildren($id, $level);
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
						$data = $this->getParents($asset->id, $level);
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
						// Remove our link
						$data = $this->removeLink($parent->id, $asset->id);
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
							$old_parent_id = $this->setAsset($_REQUEST['old_parent_id']);
							$new_parent_id = $this->setAsset($_REQUEST['new_parent_id']);
							if (!$old_parent_id && !$new_parent_id) return;
						} else {
							$this->returnError('A parent asset is required');
							return;
						}//end else
						isset($_REQUEST['link_type']) ? $link_type = $_REQUEST['link_type'] : $link_type = 1;
						isset($_REQUEST['new_position']) ? $new_position = $_REQUEST['new_position'] : $new_position = '0';
						// Remove our link
						$data = $this->moveLink($old_parent_id->id, $asset->id, $new_parent_id->id, $link_type, $new_position);
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
						isset($_REQUEST['link_type']) ? $link_type = $_REQUEST['link_type'] : $link_type = NULL;
						isset($_REQUEST['link_value']) ? $link_value = $_REQUEST['link_value'] : $link_value = NULL;
						isset($_REQUEST['sort_order']) ? $sort_order = $_REQUEST['sort_order'] : $sort_order = NULL;
						// Remove our link
						$data = $this->updateLink($parent->id, $asset->id, $link_type, $link_value, $sort_order);
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
						$all_info = (boolean)$_REQUEST['all_info'];
						// Get our link id
						$data = $this->getLinkId($parent->id, $asset->id, $all_info);
						if (!$data['link_id']) return;
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

						//let check if the status supplied is valid one
						$desc = get_status_description($status);
						$current_desc = ($status != $asset->status) ? get_status_description($asset->status) : $desc;

						$response = $this->setAssetStatus($asset, $status, $cascade, $current_desc, $desc);
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

			default:	$this->returnError('This is not a valid function name');

		}//end switch

		// Send our data as JSON
		if (isset($data)) {
			$this->returnJSON($data, $type);
		}//end if

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
				// Set our asset reference
				$asset = $am->getAssetFromURL('', strip_url($id, TRUE), TRUE, TRUE);
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
	* @param object	$asset		The asset we want to get information about
	*
	* @return array
	* @access public
	*/
	public function getGeneralInfo(Asset $asset)
	{
		if (is_null($asset)) return Array();

		//root folder?
		if ($asset->id == '1') {
			$data['name'] = 'Root Folder';
			return $data;
		}

		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$created_user = $am->getAsset($asset->created_userid);

		if (isset($asset->updated_userid) && $asset->updated_userid != '' && (int)$asset->updated_userid != 0 ) {
			$updated_user = $am->getAsset($asset->updated_userid);
		} else {
			$updated_user = 'Never Updated';
		}

		if (isset($asset->published_userid) && $asset->published_userid != '' && (int)$asset->published_userid != 0 ) {
			$published_user = $am->getAsset($asset->published_userid);
		} else {
			$published_user = 'Never Published';
		}

		if (isset($asset->status_changed_userid) && $asset->status_changed_userid != '' && (int)$asset->status_changed_userid != 0 ) {
			$status_user  = $am->getAsset($asset->status_changed_userid);
		} else {
			$status_user = 'Never Changed';
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
					'created_userid'			=>	$created_user->id,
					'created_username'			=>	$created_user->name,
					'updated'					=>	($updated_user === 'Never Updated') ? $updated_user : $asset->updated,
					'updated_userid'			=>	($updated_user === 'Never Updated') ? $updated_user : $updated_user->id,
					'update_username'			=>	($updated_user === 'Never Updated') ? $updated_user : $updated_user->name,
					'published'					=>	($published_user === 'Never Published') ? $published_user : $asset->published,
					'published_userid'			=>	($published_user === 'Never Published') ? $published_user : $published_user->id,
					'published_username'		=>	($published_user === 'Never Published') ? $published_user : $published_user->name,
					'status_changed'			=>	($status_user === 'Never Changed') ? $status_user : $asset->status_changed,
					'status_changed_userid'		=>	($status_user === 'Never Changed') ? $status_user : $status_user->id,
					'status_changed_username'	=>	($status_user === 'Never Changed') ? $status_user : $status_user->name,
				);

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
	* @param integer	$parent			Major asset id we are linking from
	* @param integer	$child			Minor asset id we are linking to
	*
	* @return void
	* @access public
	*/
	function removeLink($parent, $child)
	{
		// Set our link id
		$linkid = $this->getLinkId($parent, $child);
		if (!$linkid) return FALSE;

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
				if (count($parents) > 1) {
					// More than one parent link, we can delete the link
					$result	= $am->deleteAssetLink($linkid['link_id']);
				} else {
					// Only one link holly cow, lets trash it instead, otherwise we'l have an orphan asset in the system
					$result	= $am->trashAsset($link['minorid']);
				}//end if
				//no matter if link is deleted or the whole asset is moved to thrash, we will update lookups :)
				$result_from_update = $this->updateLookupsforAsset($child);
				if (!is_null($result_from_update)) $this->returnError('Unable to Update Lookups on the Asset #'.$child.' and its Childrens');

				// Release all our locks
				$am->releaseLock($link['minorid'], 'all');
				$am->releaseLock($link['majorid'], 'all');

				if ($result) {
					return Array (
							'success'	=> 'Asset #'.$child.' has been removed from asset #'.$parent,
						   );
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
	* @param integer  $old_parent_id		Old parent id
	* @param integer  $asset_id_to_move		Asset id to move
	* @param integer  $new_parent_id		New parent id
	* @param integer  $link_type			Type of the new link
	* @param integer  $new_position	Position under the new parent
	*
	* @return void
	* @access public
	*/
	function moveLink($old_parent_id, $asset_id_to_move, $new_parent_id, $link_type, $new_position)
	{
		if (!empty($old_parent_id) && !empty($asset_id_to_move) && !empty($new_parent_id)) {
			// Get our link id!!
			$link_id = $this->getLinkId($old_parent_id, $asset_id_to_move);
			if (!$link_id) return FALSE;
			$new_link_id = $GLOBALS['SQ_SYSTEM']->am->moveLink($link_id['link_id'], $new_parent_id, $link_type, $new_position);

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
	*
	* @return void
	* @access public
	*/
	function updateLink($parent, $child, $link_type, $link_value, $sort_order)
	{
		if (!empty($parent) && !empty($child)) {
			$link_id = $this->getLinkId($parent, $child);
			if (!$link_id) return FALSE;
			$new_link_id = $GLOBALS['SQ_SYSTEM']->am->updateLink($link_id['link_id'], $link_type, $link_value, $sort_order);

			if (!empty($new_link_id)) {
				return Array (
						'success'	=> 'Link #'.$link_id['link_id'].' has been updated',
					   );
			} else {
				//$this->returnError('Unable to update link');
				return FALSE;
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

	}//end GetPermission


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
	* @param integer	$id			Asset id of the child
	*
	* @return void
	* @access public
	*/
	function getParents($id, $level)
	{
		// Shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$parent_arr = Array();

		$parents = $am->getParents($id, '', TRUE, NULL, NULL, TRUE, NULL, $level);
		foreach ($parents as $parent => $type_code) {
			// Levels greater than 1 won't have a link id with the parent
			if (!$this->hasLink($parent, $id)) {
				$show['link_id']		= NULL;
				$show['is_dependant']	= NULL;
				$show['is_exclusive']	= NULL;
				$show['sort_order']		= NULL;
				$show['link_value']		= NULL;
			} else {
				$show_link = $this->getLinkId($parent, $id, TRUE);
				$show['link_id']		= $show_link['link_id'];
				$show['is_dependant']	= $show_link['is_dependant'];
				$show['is_exclusive']	= $show_link['is_exclusive'];
				$show['sort_order']		= $show_link['sort_order'];
				$show['link_value']		= $show_link['value'];
			}
			$asset = $am->getAsset($parent);
			$parent_arr[$asset->id] = array_merge($this->getGeneralInfo($asset), $show);

		}//end foreach

		return $parent_arr;

	}//end getParents


	/**
	* Description: Gets child asset ids
	*
	* @param integer	$id			Asset id of the parent
	* @param integer	$level		How deep to go
	*
	* @return void
	* @access public
	*/
	function getChildren($id, $level)
	{
		// Shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$children_arr =Array();

		$children = $am->getChildren($id, '', TRUE, NULL, NULL, NULL, TRUE, NULL, $level);
		foreach ($children as $children => $child_id) {
			$child = $am->getAsset($children);
			// Levels lower than 1 won't have a link id with the parent
			if (!$this->hasLink($id, $children)) {
				$show['link_id']		= NULL;
				$show['is_dependant']	= NULL;
				$show['is_exclusive']	= NULL;
				$show['sort_order']		= NULL;
				$show['link_value']		= NULL;
			} else {
				$show_link = $this->getLinkId($id, $child->id, TRUE);
				$show['link_id']		= $show_link['link_id'];
				$show['is_dependant']	= $show_link['is_dependant'];
				$show['is_exclusive']	= $show_link['is_exclusive'];
				$show['sort_order']		= $show_link['sort_order'];
				$show['link_value']		= $show_link['value'];
			}
			$children_arr[$child->id] = array_merge($this->getGeneralInfo($child), $show);

		}//end foreach

		return $children_arr;

	}//end getChildren


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
				//done thrashing the asset? update lookups then
				$result_from_update = $this->updateLookupsforAsset($id);
				if (!is_null($result_from_update)) $this->returnError('Unable to Update Lookups on the Asset #'.$id.' and its Childrens');
			}//end else
			$am->releaseLock($id, 'all');

		} else {
			$this->returnError('Asset #'.$id.' is not valid. Please provide a valid Asset ID');
			return;
		}//end else

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
			for ($i = 0; $i < count($fieldid); $i++ ) {

				$field	= $am->getAsset($fieldid[$i]);
				if (!($field instanceof Metadata_Field)) {
					$this->returnError('The Field ID provided does not belong to a metadata field');
				}//end if

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

			// instead for doing it for every field, lets regen content file just once here
			if (!$mm->generateContentFile($assetid)) $g_result = FALSE;
			if ($s_result && $g_result) {
				return Array (
						'success'	=> 'The metadata has been correctly set',
					   );
			} else {
				$this->returnError('Unable To Regenerate Metadata For Asset');
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
	* @param boolean	$all_info		If to return just LinkId or all the information
	*
	* @return string/array
	* @access public
	*/
	function getLinkId($parent, $child, $all_info = FALSE)
	{
		if (!empty($parent) && !empty($child)) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent, $child);
			if (empty($link)) {
				$this->returnError('Parent #'.$parent.' and child #'.$child.' do not have a valid link');
				return FALSE;
			}
			if (!$all_info) {
				$link_array['link_id'] = array_get_index($link, 'linkid', 0);
				return $link_array;
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
			$init_hipo =& new Hipo_Job_Acquire_Locks();
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
	public function setAssetStatus(Asset $asset, $status, $cascade, $current_status_desc, $to_be_status_desc)
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
		$init_hipo =& new HIPO_Job_Edit_Status();
		$running_vars = Array(
                            'assetid'           => $asset->id,
                            'new_status'        => $status,
                            'dependants_only'   => !$cascade,
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
				echo $output;

			}//end if

		}//end if

	}//end returnJSON()


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
	if (typeof(obj) == 'undefined') {
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

	// Make our request
	makeRequest(url, true, dataCallback);

}//end trashAsset


/**
* This will return child asset ids of the passed asset
*
* @param string		asset_id		Id of the asset to get children of
* @param number		levels			Number of levels to return
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getChildren(asset_id, levels, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set any levels
	var levels = typeof(levels) != 'undefined' ? levels : 0;

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getChildren&id=' + asset_id + '&depth=' + levels;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getChildren


/**
* This return parents of the passed id
*
* @param string		asset_id		Id of the asset to get parents of
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getParents(asset_id, levels, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set any levels
	var levels = typeof(levels) != 'undefined' ? levels : 0;

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getParents&id=' + asset_id + '&depth=' + levels;

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
	if (!isset(dependants_only)) dependants_only = '1';
	if (!isset(force_acquire)) force_acquire = '0';
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

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
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function removeLink(parent_id, child_id, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=removeLink&id=' + child_id + '&parent_id=' + parent_id;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end removeLink


/**
* Moves a link from one parent to another
*
* @param string		old_parent_id			Id of the old parent
* @param string		child_id				Id of the child
* @param string		new_parent_id			Id of the new parent
* @param string		link_type				Type of link to use
* @param string		new_position			The new position
* @param function	dataCallback			Custom callback function
*
* @version $Revision: 0.2
*/
function moveLink(old_parent_id, child_id, new_parent_id, link_type, new_position, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	if (!isset(link_type)) link_type = '1';
	if (!isset(new_position)) new_position = '0';

	// Build our string
	var url =	'<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=moveLink' + 
				'&id=' + child_id +
				'&old_parent_id=' + old_parent_id +
				'&new_parent_id=' + new_parent_id +
				'&link_type=' + link_type +
				'&new_position=' + new_position;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end moveLink


/**
* Updates a link
*
* @param string		parent_id				Id of the parent
* @param string		child_id				Id of the child
* @param string		new_parent_id			Id of the new parent
* @param string		link_type				Type of link to use
* @param string		new_position			The new position
* @param function	dataCallback			Custom callback function
*
* @version $Revision: 0.2
*/
function updateLink(parent_id, child_id, link_type, link_value, sort_order, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	if (!isset(link_type)) link_type = '';
	if (!isset(sort_order)) sort_order = '';

	// Build our string
	var url =	'<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=updateLink' + 
				'&id=' + child_id +
				'&parent_id=' + parent_id +
				'&link_type=' + link_type +
				(isset(link_value) ? '&link_value=' + link_value : '') +
				'&sort_order=' + sort_order;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end updateLink


/**
* Returns the link id between a parent and child
*
* @param string		parent_id		Id of the parent
* @param string		child_id		Id of the child
* @param function	all_info		if we want all the link information or just linkid
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getLinkId(parent_id, child_id, all_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var all_info = isset(all_info) ? all_info : 0;

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getLinkId&id=' + child_id + '&parent_id=' + parent_id + '&all_info=' + all_info;

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
* @param string		asset_id		Id of the asset to get children of
* @param int		status			status tha asset is to be set to
* @param boolean	cascade			if to cascade the status to non-dependant children
* @param function	dataCallback	Custom callback function
*
*
*/
function setAssetStatus(asset_id, status, cascade, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setAssetStatus&id=' + asset_id + '&status=' + status + '&cascade=' + cascade;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setAssetStatus()


	<?php
	}//end printJs


}//end class
?>
