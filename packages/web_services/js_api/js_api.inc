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
* $Id: js_api.inc,v 1.2.2.12 2010/03/26 04:45:11 cupreti Exp $
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
* @version $Revision: 1.2.2.12 $
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
						// Get general info about asset
						$data = Array(
									'name'					=>	$asset->name,
									'short_name'			=>	$asset->short_name,
									'id'					=>	$asset->id,
									'type_code'				=>	$asset->type(),
									'icon_path'				=>	$am->getAssetIconURL($asset->type()),
									'data_path'				=>	$asset->data_path,
									'web_path'				=>	$asset->getURL(),
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

			case	'setAttribute':
				if ($this->attr('set_attribute')) {
					if ($wa) {
						if (isset($_REQUEST['attr_name']) && isset($_REQUEST['attr_val'])) {
							// Check to see if the passed name is even an attribute
							$attributes = $am->getAssetTypeAttributes($asset->type());
							if (array_key_exists($_REQUEST['attr_name'], $attributes)) {
								// Set our new values
								$this->setAttributeValue($asset->id, $_REQUEST['attr_name'], str_replace($replace_these, $replace_with, $_REQUEST['attr_val']));
								$data['success'] = 'Attribute '.$_REQUEST['attr_name'].' has been set to '.str_replace($replace_these, $replace_with, $_REQUEST['attr_val']);
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
						$data = $mm->getMetadataFieldValues($asset->id);
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
							$this->setAssetMetadata($asset->id, $_REQUEST['field_id'], str_replace($replace_these, $replace_with, $_REQUEST['field_val']));
							$data['success'] = 'Metadata field #'.$_REQUEST['field_id'].' has been set to: '.str_replace($replace_these, $replace_with, $_REQUEST['field_val']);
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
						$data = $this->getParents($asset->id);
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
							isset($_REQUEST['asset_name']) ? $asset_name = str_replace($replace_these, $replace_with,  $_REQUEST['asset_name']) : $asset_name = '';
							isset($_REQUEST['link_type']) ? $link_type = $_REQUEST['link_type'] : $link_type = 1;
							isset($_REQUEST['link_value']) ? $link_value = str_replace($replace_these, $replace_with,  $_REQUEST['link_value']) : $link_value = '';
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

			case	'acquireLock':
				if ($this->attr('acquire_lock')) {
					if ($wa) {
						$screen = $_REQUEST['screen'];
						$response = $am->acquireLock($asset->id, $screen);
						if ($response) {
							$data = Array('Asset #'.$asset->id.' lock\'s have been acquired');
						} else {
							$this->returnError('You cannot acquire locks on this asset for this screen');
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
						$response = $am->releaseLock($asset->id, $screen);
						if ($response) {
							$data = Array('Asset #'.$asset->id.' lock\'s have been released');
						} else {
							$this->returnError('You cannot release locks on this asset');
						}// end if
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
						// Get our link id
						$data['link_id'] = $this->getLinkId($parent->id, $asset->id);
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

		if ($linkid != '') {
			$link = $am->getLinkById($linkid);
			// Make sure our array contains some items
			if (!empty($link)) {
				// Get all our locks
				$am->acquireLock($link['majorid'], 'all');
				$am->acquireLock($link['minorid'], 'all');
				$parents = $am->getLinks($link['minorid'], SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');

				// See if we should delete a link or move the asset to the trash
				if (count($parents) > 1) {
					// More than one parent link, we can delete the link
					$result	= $am->deleteAssetLink($linkid);
				} else {
					// Only one link holly cow, lets trash it instead, otherwise we'l have an orphan asset in the system
					$result	= $am->trashAsset($link['minorid']);
				}//end if

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
			$new_link_id = $GLOBALS['SQ_SYSTEM']->am->moveLink($link_id, $new_parent_id, $link_type, $new_position);

			if (!empty($new_link_id)) {
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
			$new_link_id = $GLOBALS['SQ_SYSTEM']->am->updateLink($link_id, $link_type, $link_value, $sort_order);

			if (!empty($new_link_id)) {
				return Array (
						'success'	=> 'Link #'.$link_id.' has been updated',
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
	function getParents($id)
	{
		// Shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$parents = $am->getParents($id, '', TRUE, NULL, NULL, TRUE, NULL, 1);
		foreach ($parents as $parent => $type_code) {
			$asset = $am->getAsset($parent);
			$parent_arr[$asset->id] = Array(
							'id'		=>	$asset->id,
							'name'		=>	$asset->name,
							'type_code'	=>	$asset->type(),
							'link_id'	=>	$this->getLinkId($asset->id, $id),
						);
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

		$children = $am->getChildren($id, '', TRUE, NULL, NULL, NULL, TRUE, NULL, $level);
		foreach ($children as $children => $child_id) {
			$child = $am->getAsset($children);
			// Levels lower than 1 won't have a link id with the parent
			if (!$this->hasLink($id, $child->id)) {
				$show_link_id = NULL;
			} else {
				$show_link_id = $this->getLinkId($id, $child->id);
			}
			$children_arr[$child->id] = Array(
							'id'		=>	$child->id,
							'name'		=>	$child->name,
							'type_code'	=>	$child->type(),
							'link_id'	=>	$show_link_id,
						);
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
	* @param string		$attr_name	The name of the attribute
	* @param string		$attr_val	The new value of the attribute
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
			$this->returnError('Please make sure to provide an Asset ID, Attribute Name and Attribute Value');
		}//end else

	}//end setAttributeValue()


	/**
	* Description: This operation will set the value for a metadata field of an asset
	*
	* @param string		$assetid	The asset to regenerate metadata for
	* @param string		$fieldid	The metadata field id
	* @param string		$new_value	The new value for the field
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
	*
	* @return void
	* @access public
	*/
	function getLinkId($parent, $child)
	{
		if (!empty($parent) && !empty($child)) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent, $child);
			$linkid = array_get_index($link, 'linkid', 0);

			if (empty($linkid)) {
				$this->returnError('Parent #'.$parent.' and child #'.$child.' do not have a valid link');
				return FALSE;
			}

			return $linkid;
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
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setAttribute&id=' + asset_id + '&attr_name=' + attr_name + '&attr_val=' + attr_val.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setAttribute


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
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=setMetadata&id=' + asset_id + '&field_id=' + field_id + '&field_val=' + field_val.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setMetadata


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
function getParents(asset_id, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getParents&id=' + asset_id;

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
* Acquires a lock
*
* @param string		asset_id		Id of the asset to get locks for
* @param string		screen_name		The screen to get locks for
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function acquireLock(asset_id, screen_name, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// If the user does not set it, we get all locks
	if (!isset(screen_name) || screen_name == '') screen_name = 'all';

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=acquireLock&id=' + asset_id + '&screen=' + screen_name;

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
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getLinkId(parent_id, child_id, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $this->getURL(); ?>' + '?key=' + api_key + '&type=getLinkId&id=' + child_id + '&parent_id=' + parent_id;

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

	<?php
	}//end printJs


}//end class
?>
