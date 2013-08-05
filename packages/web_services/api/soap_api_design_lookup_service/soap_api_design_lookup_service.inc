<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_design_lookup_service.inc,v 1.11.6.2 2013/08/05 07:26:04 cupreti Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* Soap_Api_Design_Lookup_Service
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.11.6.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_Design_Lookup_Service extends Soap_Api
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns a function list of the asset let the children handle this
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    public function getFunctionList()
    {
        return Array(
				'ApplyDesign'			=> '1',
				'RemoveDesign'			=> '1',
				'GetDesignFromURL'		=> '1',
				'ApplyAssetPaintLayout'	=> '1',
        		'RemoveAssetPaintLayout'=> '1',
				'GetPaintLayoutFromURL'	=> '1',
			   );

    }//end getFunctionList()


	/**
	* Description: This operation will apply a design to an asset
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		 'DesignID'	 				=> [The Design ID],
	*		 'AssetID'			 		=> [The AssetID of the asset being applied the design],
	*		 'DesignType'				=> [Type of Design: Frontend, OverrideFrontend, Login, OverrideLogin, UserDefined, OverrideUserDefined],
    *        'UserDefinedDesignName'    => [Name of the new User Defined design],

    *        )
    * </pre>
	*
	* @return string
	* @access public
	*/
	function ApplyDesign($request)
	{
		$request_info				= (Array) $request;
		$new_designid				= array_get_index($request_info, 'DesignID', '');
		$assetid					= array_get_index($request_info, 'AssetID', '');
		$design_type				= array_get_index($request_info, 'DesignType', '');
		$user_defined_design_name	= array_get_index($request_info, 'UserDefinedDesignName', '');

		$design_name	= self::getDesignNameFromType($design_type);
		if (empty($design_name)) throw new SoapFault('Server', 'The Design Type is invalid. Operation Aborted.');

		$designs_ok		= TRUE;
		$design_changed	= FALSE;

		if (!empty($assetid) && !empty($new_designid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$design	= $GLOBALS['SQ_SYSTEM']->am->getAsset($new_designid);

			if (!($design instanceof Design)) {
				throw new SoapFault('Server', 'The DesignID Provided Does Not Belong To A Design Asset');
			}//end if

        	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	        $GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$update_design_cascade = FALSE;
	        if ($design_name == ('design::user::'.$user_defined_design_name)) {
	        	$old_designid = 0;
	        	if (empty($user_defined_design_name)) {
	        		throw new SoapFault('Server', 'User Defined Design Name is either empty or not valid.');
	        	} else {
	                if ($GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'design', FALSE, $user_defined_design_name)) {
	                	throw new SoapFault('Server', 'Design Name '.$user_defined_design_name.' is already in use');
	                    trigger_localised_error('SYS0134', E_USER_WARNING, $user_defined_design_name);
	                }// end if
	        	}//end else
	        } else {
	        	// Is it not user defined design
				if (in_array($design_type, Array('OverrideUserDefined', 'UserDefined'))) {
		        		$design_name	= $design_name.$user_defined_design_name;
		        }//end if

				$old_design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'design', FALSE, $design_name);
				$old_designid    = (empty($old_design_link)) ? 0 : $old_design_link['minorid'];

	        }//end else

			if ($new_designid != $old_designid) {

				if ($old_designid) {
                    if (!$asset->deleteLink($old_design_link['linkid'])) {
                        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
                        $designs_ok = FALSE;
                        break;
                    }//end if
				}//end if

                if ($new_designid) {
                    $design = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_designid);
                    if (is_null($design)) {
                        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
                        $designs_ok = FALSE;
                        break;
                    }//end if

                    if (!$asset->createLink($design, SQ_LINK_NOTICE, $design_name)) {
                        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
                        $designs_ok = FALSE;
                        break;
                    }//end if
                }//end if
                $update_design_cascade  = TRUE;
			} else {
				throw new SoapFault('Server', 'The Design Has Already Been Applied To The Asset. Operation Aborted.');
			}//end if
            $designs_changed = TRUE;

	        if ($designs_ok) {
	            if ($designs_changed) {
	                if ($update_design_cascade == TRUE) {
	                    $assets_affected_lookups[] = $asset->id;
	                }//end if
	            }//end if
	            $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	        }//end if

	        $GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			$update_lookup_result	= $asset->updateLookups();

			if (!$update_lookup_result) {
				throw new SoapFault('Server', 'Update Lookups For Asset Failed. Try Running Update Lookups Again Another Time');
			}//end if

			return Array (
					'ApplyDesignResult'	=> $designs_changed,
				   );

		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end ApplyDesign()


	/**
	* This function return the according design name for a design type
	*
	* @param $design_type	string	The design type, refer to apply and remove design function
	*
	* @return string
	* @access private
	*/
	private static function getDesignNameFromType($design_type)
	{
		$design_name	= '';
		switch ($design_type) {
			case 'OverrideFrontend':
				$design_name	= 'override::design::system::frontend';
				break;
			case 'OverrideLogin':
				$design_name	= 'override::design::system::login';
				break;
			case 'OverrideUserDefined':
				$design_name	= 'override::design::user::';
				break;
			case 'Frontend':
				$design_name	= 'design::system::frontend';
				break;
			case 'Login':
				$design_name	= 'design::system::login';
				break;
			case 'UserDefined':
				$design_name	= 'design::user::';
				break;
			default:
				break;
		}//end switch
		return $design_name;

	}//end getDesignNameFromType()


	/**
	* Description: This operation will remove a design to an asset
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		 'DesignID'	 				=> [The Design ID],
	*		 'AssetID'			 		=> [The AssetID of the asset being applied the design],
	*		 'DesignType'				=> [Type of Design: Frontend, OverrideFrontend, Login, OverrideLogin, UserDefined, OverrideUserDefined],
    *        'UserDefinedDesignName'    => [Name of the new User Defined design],

    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function RemoveDesign($request)
	{
		$request_info				= (Array) $request;
		$design_id					= array_get_index($request_info, 'DesignID', '');
		$assetid					= array_get_index($request_info, 'AssetID', '');
		$design_type				= array_get_index($request_info, 'DesignType', '');		// ( frontend || user-defined || login ) || override
		$user_defined_design_name	= array_get_index($request_info, 'UserDefinedDesignName', '');


		if (!empty($assetid) && !empty($design_id)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$design	= $GLOBALS['SQ_SYSTEM']->am->getAsset($design_id);

			$design_name	= self::getDesignNameFromType($design_type);
			$designs_ok		= TRUE;

			if (!($design instanceof Design)) {
				throw new SoapFault('Server', 'The DesignID Provided Does Not Belong To A Design Asset');
			}//end if

        	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	        $GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	        	// Is it not user defined design
				if (in_array($design_type, Array('OverrideUserDefined', 'UserDefined'))) {
		        		$design_name	= $design_name.$user_defined_design_name;
		        }//end if

				$old_design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'design', FALSE, $design_name);
				$old_designid    = (empty($old_design_link)) ? 0 : $old_design_link['minorid'];

				if ($old_designid && $old_designid == $design_id) {
                    if (!$asset->deleteLink($old_design_link['linkid'])) {
                        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
                        $designs_ok = FALSE;
                        break;
                    }//end if
				} else {
					throw new SoapFault('Server', 'No Such Design Found For Asset');
				}//end else

	        if ($designs_ok) {
	            $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	        }//end if

			$update_lookup_result	= $asset->updateLookups();

			if (!$update_lookup_result) {
				throw new SoapFault('Server', 'Update Lookups For Asset Failed. Try Running Update Lookups Again Another Time');
			}//end if

			return Array (
					'RemoveDesignResult'	=> $designs_ok,
				   );

		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end RemoveDesign()


	/**
	* Description: This operation returns the effective design of an URL
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'URL'					=> [The URL in question],
	*		'DesignType'			=> [Type of Design: Frontend, OverrideFrontend, Login, OverrideLogin, UserDefined, OverrideUserDefined],
	*		'UserDefinedDesignName'	=> [Name of the new User Defined design],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetDesignFromURL($request)
	{
		$request_info				= (Array) $request;
		$url						= array_get_index($request_info, 'URL', '');
		$design_type				= array_get_index($request_info, 'DesignType', '');		// ( frontend || user-defined || login ) || override
		$user_defined_design_name	= array_get_index($request_info, 'UserDefinedDesignName', '');

		// Design is associated with the domain bit only (i.e. url without protocol)
		$url = str_replace('http://' , '', $url);
		$url = str_replace('https://', '', $url);

		$design_name	= self::getDesignNameFromType($design_type);
        // Is it not user defined design
		if (in_array($design_type, Array('OverrideUserDefined', 'UserDefined'))) {
        		$design_name	= $design_name.$user_defined_design_name;
        }//end if

		if (empty($design_name)) {
			$design_name	= 'design::system::frontend';
		}//end if

		$designs		= $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, $design_name);
		$design_result	= Array (
							'DesignID'	=> $designs['designid'],
							'TypeCode'	=> $designs['type_code'],
						  );

		return Array (
				'GetDesignFromURLResult'	=> $design_result,
			   );

	}//end GetDesignFromURL()


	/**
	* Description: This operation will apply a paintlayout to an asset
	*
	* @param array $request		the request information
    * <pre>
    * Array (
    *		'PaintLayoutID'		=> [The assetid of the paintlayout],
	*		'AssetID'			=> [The asset being applied paintlayout to],
	*		'PaintLayoutType'	=> [Type of paint layout Frontend or OverrideFrontend or UserDefined],
	*		'AssetUrl'			=> [The asset url to apply the paint layout to (url specific)],
	*		'PaintLayoutCode'	=> [Layout code for the user defined layout],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function ApplyAssetPaintLayout($request)
	{
		$request_info				= (Array) $request;
		$paint_layout_id			= array_get_index($request_info, 'PaintLayoutID', '');
		$assetid					= array_get_index($request_info, 'AssetID', '');
		$paint_layout_type			= array_get_index($request_info, 'PaintLayoutType', '');		// ( Frontend | OverrideFrontend | UserDefined )
		$asset_url					= array_get_index($request_info, 'AssetUrl', '');
		$paint_layout_code			= array_get_index($request_info, 'PaintLayoutCode', '');

		if (!empty($assetid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (empty($paint_layout_id)) {
				throw new SoapFault('Server', 'The PaintLayoutID provided is invalid');
			}//end if

			$result	= self::changePaintLayout($asset, $paint_layout_id, $paint_layout_type, $asset_url, $paint_layout_code);

			return Array (
					'ApplyAssetPaintLayoutResult'	=> $result,
				   );

		} else {
			throw new SoapFault('Server', 'The AssetID supplied is either empty or not valid');
		}//end else

	}//end ApplyPaintLayout()


	/**
	* Description: This operation will remove a paintlayout to an asset
	*
	* @param array $request		the request information
    * <pre>
    * Array (
    *		'PaintLayoutID'		=> [The assetid of the paintlayout],
	*		'AssetID'			=> [The asset being remove paintlayout from],
	*		'PaintLayoutType'	=> [Type of paint layout Frontend or OverrideFrontend or UserDefined],
	*		'AssetUrl'			=> [The asset url to apply the paint layout to (url specific)],
	*		'PaintLayoutCode'	=> [Layout code for the user defined layout],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function RemoveAssetPaintLayout($request)
	{
		$request_info				= (Array) $request;
		$paint_layout_id			= array_get_index($request_info, 'PaintLayoutID', '');
		$assetid					= array_get_index($request_info, 'AssetID', '');
		$paint_layout_type			= array_get_index($request_info, 'PaintLayoutType', '');	// ( Frontend | OverrideFrontEnd | UserDefined)
		$asset_url					= array_get_index($request_info, 'AssetUrl', '');
		$paint_layout_code			= array_get_index($request_info, 'PaintLayoutCode', '');

		if (empty($assetid)) {
			throw new SoapFault('Server', 'The AssetID supplied is either empty or not valid');
		}
		$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

		if ($paint_layout_type == 'UserDefined' && empty($asset_url)) {
			throw new SoapFault('Server', 'Cannot apply the user defined paint layout. Asset URL not specifed');
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all');

		$result	= TRUE;

		if (empty($asset_url) && $paint_layout_type != 'UserDefined') {

			// We should check whether they are telling the truth
			$paint_layout_end_name	= '::system::frontend';
			$value_name				= $paint_layout_type == 'Frontend' ? 'paint_layout' : 'override::paint_layout';
			$paint_layout_name		= $value_name.$paint_layout_end_name;
			$existing_layout_link	= $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'paint_layout_page', TRUE, $paint_layout_name);
			if (empty($existing_layout_link) || ($existing_layout_link['minorid'] != $paint_layout_id)) {
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
				throw new SoapFault('Server', 'The asset does not have a paint layout applied with the provided paint layout ID');
			}//end if

			$paint_layout_end_name	= '::system::frontend';
			$value_name				= $paint_layout_type == 'Frontend' ? 'paint_layout' : 'override::paint_layout';
			$paint_layout_name		= $value_name.$paint_layout_end_name;

			// All we are going to do in here is to delete the link, delete lookup value and then run update lookup
			$layouts_changed = FALSE;

			// First we are going to get the link
			$existing_layout_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'paint_layout_page', TRUE, $paint_layout_name);

			// If we have hte link, lets delete it
			if ($existing_layout_link) {
				if (!$asset->deleteLink($existing_layout_link['linkid'])) {
					$result	= FALSE;
				} else {
					$layouts_changed = TRUE;
				}//end if

				// also need to delete any other URLs which use the same value
				// deleting an asset based value means all other URLs which use the same value are also deleted
				if ($result && !$new_values = $asset->deleteLookupValue($paint_layout_name, $existing_layout_link['minorid'])) {
					$result = FALSE;
				}//end if
			}//end if

			if ($result && $layouts_changed && !$asset->updateLookups()) {
				$result = FALSE;
			}//end if
		} else {
			// We need to remove URL based paint layouts, including the user defined
			if ($paint_layout_type == 'UserDefined') {
				if (empty($paint_layout_code)) {
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
					throw new SoapFault('Server', 'The layout code not specified for the User Defined paint layout');
				}
				$paint_layout_name = 'paint_layout::user::'.$paint_layout_code;
			} else {
				$paint_layout_name = 'paint_layout::system::frontend';
			}

			// Validate the specifed URL
			$asset_url = rtrim($asset_url, '/');
			$valid_url = FALSE;
			$asset_urls = $GLOBALS['SQ_SYSTEM']->am->getURLs($asset->id);
			foreach($asset_urls as $url_info) {
				if ($url_info['url'] == $asset_url) {
					$valid_url = TRUE;
					break;
				}//end if
			}//end foreach
			if (!$valid_url) {
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
				throw new SoapFault('Server', 'The specifed URL '.$asset_url.' does not belongs to the asset #'.$asset->id);
			}

			$lookup_type = $paint_layout_type == 'OverrideFrontend' ? $asset_url.'/' : $asset_url;

			// Get existing lookup values set on the asset
			$lookup_values = $asset->getLookupValues(FALSE);
			if (isset($lookup_values[$lookup_type][$paint_layout_name]['value']) && $lookup_values[$lookup_type][$paint_layout_name]['value'] ==$paint_layout_id) {
				unset($lookup_values[$lookup_type][$paint_layout_name]);
				if (!$asset->setLookupValues($lookup_values)) {
					$result =  FALSE;
				}
			} else {
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
				$url_based_type = $paint_layout_type == 'UserDefined' ? 'user defined' : 'URL based';
				throw new SoapFault('Server', 'The specifed '.$url_based_type.' paint layout #'.$paint_layout_id.' is not applied the asset #'.$asset->id);
			}
		}//end else

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
		$GLOBALS['SQ_SYSTEM']->doTransaction($result ? 'COMMIT' : 'ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


		return Array (
					'RemoveAssetPaintLayoutResult'	=> $result,
				   );

	}//end RemoveAssetPaintLayout()


	/**
	* This function will apply paint layout to an asset, refer to Lookup Setting screen
	*
	* @param $asset				object	The asset being applied paintlayout
	* @param $paint_layout_id	string	The assetid of the paintlayout
	* @param $paint_layout_type	string	The paint layout type
	* @param $asset_url			string	The asset url to apply the paint layout to (url specific)
	* @param $paint_layout_code	string	The layout code for user defined layout
	*
	* @return boolean
	* @access private
	*/
	private static function changePaintLayout($asset, $paint_layout_id, $paint_layout_type, $asset_url='', $paint_layout_code='')
	{
		$paint_layout			= NULL;
		// We are removing asset based paint layout
		if ($paint_layout_id != 0) {
			$paint_layout			= $GLOBALS['SQ_SYSTEM']->am->getAsset($paint_layout_id);
			if (!($paint_layout instanceof Paint_Layout_Page)) {
				throw new SoapFault('Server', 'The paint layout ID provided does not belong to a paint layout page asset');
			}//end if
		}//end if

		if ($paint_layout_type == 'UserDefined' && empty($asset_url)) {
			throw new SoapFault('Server', 'Cannot apply the user defined paint layout. Asset URL not specifed');
		}

		// Get existing lookup values set on the asset
		$lookup_values = $asset->getLookupValues(FALSE);

    	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all');

		$result = TRUE;

		if (!empty($asset_url)) {
			if ($paint_layout_type == 'UserDefined') {
				if (empty($paint_layout_code)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all');
					throw new SoapFault('Server', 'The layout code not specified for the User Defined paint layout');
				}
				$paint_layout_name = 'paint_layout::user::'.$paint_layout_code;
			} else {
				$paint_layout_name = 'paint_layout::system::frontend';
			}

			// Validate the specifed URL
			$asset_url = rtrim($asset_url, '/');
			$valid_url = FALSE;
			$asset_urls = $GLOBALS['SQ_SYSTEM']->am->getURLs($asset->id);
			foreach($asset_urls as $url_info) {
				if ($url_info['url'] == $asset_url) {
					$valid_url = TRUE;
					break;
				}//end if
			}//end foreach
			if (!$valid_url) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all');
				throw new SoapFault('Server', 'The specifed URL '.$asset_url.' does not belongs to the asset #'.$asset->id);
			}

			$lookup_type = $paint_layout_type == 'OverrideFrontend' ? $asset_url.'/' : $asset_url;

		} else if ($paint_layout_type != 'UserDefined') {
			$lookup_type = 'asset';
			$paint_layout_name = ($paint_layout_type == 'Frontend' ? 'paint_layout' : 'override::paint_layout').'::system::frontend';

			$existing_layout_link	= $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'paint_layout_page', TRUE, $paint_layout_name);
			$layouts_changed		= FALSE;

			$create_new	= $delete_existing	= FALSE;
			if (empty($existing_layout_link)) {
				$create_new	= TRUE;
				// We are creating new layout notice link
			} else if ($existing_layout_link['minorid'] !== $paint_layout_id) {
				$delete_existing	= TRUE;
				$create_new			= TRUE;
			}//end else

			if ($delete_existing) {
				if ($existing_layout_link) {
					if (!$asset->deleteLink($existing_layout_link['linkid'])) {
						$result = FALSE;
					} else {
						$layouts_changed = TRUE;
					}//end if

					// also need to delete any other URLs which use the same value
					// deleting an asset based value means all other URLs which use the same value are also deleted
					if ($result && !$new_values = $asset->deleteLookupValue($paint_layout_name, $existing_layout_link['minorid'])) {
						$result = FALSE;
					} else {
						$lookup_values = $new_values;
						unset($new_values);
					}//end if
				}//end if
			}//end if

			if ($result && $create_new) {
				// create new notice link
				if (is_null($paint_layout)) {
					$result = FALSE;
				} else if (!$asset->createLink($paint_layout, SQ_LINK_NOTICE, $paint_layout_name)) {
					$result = FALSE;
				} else {
					$layouts_changed = TRUE;
				}//end else
			}//end if
		}

		if (empty($paint_layout_id)) {
		    if (isset($lookup_values[$lookup_type][$paint_layout_name])) {
		        unset($lookup_values[$lookup_type][$paint_layout_name]);
		    }//end if

		    // if value is deleted, run updateLookups() so the asset based definition can take effect if there is one defined
		    $layouts_changed = TRUE;
		} else {
		    if (!isset($lookup_values[$lookup_type])) {
		        $lookup_values[$lookup_type] = Array();
		    }//end if
		    $lookup_values[$lookup_type][$paint_layout_name] = Array('value' => $paint_layout_id);
		}//end else

		if ($result && !$asset->setLookupValues($lookup_values)) {
			$result = FALSE;
		}//end if

		if ($result && $layouts_changed && !$asset->updateLookups()) {
			$result = FALSE;
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction($result ? 'COMMIT' : 'ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');

		return TRUE;

	}//end changePaintLayout()


	/**
	* Description: This operation returns the paint layout applied to the given url
	*
	* @param array $request		the request information
    * <pre>
    * Array (
    *		'URL'					=> [The URL in question],
	*		'PaintLayoutType'		=> [Type of paint layout applied: Frontend or OverrideFrontend or UserDefined],
	*		'PaintLayoutCode'		=> [Layout code for the user defined layout],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetPaintLayoutFromURL($request)
	{
		$request_info = (Array) $request;
		$url = array_get_index($request_info, 'URL', '');
		$layout_type = array_get_index($request_info, 'PaintLayoutType', '');
		$layout_code = array_get_index($request_info, 'PaintLayoutCode', '');

		$valid_types = Array('Frontend', 'OverrideFrontend', 'UserDefined');

		// Input validation
		if (empty($url) || empty($layout_type)) {
			throw new SoapFault('Server', 'Value for required parameters "URL" and/or "PaintLayoutType" not specifed');
		} else if (!in_array($layout_type, $valid_types)) {
			throw new SoapFault('Server', 'Value for parameter "PaintLayoutType" needs be either "'.implode('" or "', $valid_types).'"');
		}
		if ($layout_type == 'UserDefined' && empty($layout_code)) {
			throw new SoapFault('Server', 'The layout code not specified for the User Defined paint layout');
		}

		// Get rid of protocol bit from the url
		$url = str_replace('http://' , '', $url);
		$url = str_replace('https://', '', $url);
		$url = rtrim($url, '/'); // get rid of trailing slash if any

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, $url);
		if (is_null($asset)) {
			throw new SoapFault('Server', 'Specifed URL does not belongs to any asset');
		}

		if ($layout_type == 'UserDefined') {
			$layout_name = 'paint_layout::user::'.$layout_code;
		} else {
			$layout_name = 'paint_layout::system::frontend';
		}

		if ($layout_type == 'OverrideFrontend') {
			$url = $url.'/';
		}

		$lookup_values = $asset->getLookupValues(TRUE);
		if (isset($lookup_values[$url][$layout_name]['value'])) {
			$layout_id = $lookup_values[$url][$layout_name]['value'];
		} else {
			$layout_id = NULL;
		}

		return Array (
				'AssetID'	=> $layout_id,
			   );

	}//end GetPaintLayoutFromURL()


	/**
	* getComplexElements
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$obj_optional	= self::getArgType('AssetObject', 0, 1);
		$design_link_summary	= self::getArgType('DesignLinkSummary', 0, 1);

		$complex_types	= Array (
							'ApplyDesign'			=> Array (
														'ApplyDesign'				=> Array (
																						'DesignID'				=> self::$string_non_optional,
																						'AssetID'				=> self::$string_non_optional,
																						'DesignType'			=> self::$string_non_optional,
																						'UserDefinedDesignName'	=> self::$string_optional,
																					   ),
														'ApplyDesignResponse'		=> Array (
																						'ApplyDesignResult'		=> self::$boolean_optional,
																					   ),
													   ),
							'RemoveDesign'			=> Array (
														'RemoveDesign'				=> Array (
																						'DesignID'				=> self::$string_non_optional,
																						'AssetID'				=> self::$string_non_optional,
																						'DesignType'			=> self::$string_optional,
														   								'UserDefinedDesignName'	=> self::$string_optional,
																					   ),
														'RemoveDesignResponse'		=> Array (
																						'RemoveDesignResult'	=> self::$boolean_optional,
																					   ),
													   ),
							'GetDesignFromURL'		=> Array (
														'GetDesignFromURL'			=> Array (
																						'URL'					=> self::$string_non_optional,
																						'DesignType'			=> self::$string_optional,
														   								'UserDefinedDesignName'	=> self::$string_optional,
																					   ),
														'GetDesignFromURLResponse'	=> Array (
																						'GetDesignFromURLResult'	=> $design_link_summary,
																					   ),
													   ),
							'ApplyAssetPaintLayout'	=> Array (
														'ApplyAssetPaintLayout'		=> Array (
																						'PaintLayoutID'			=> self::$string_non_optional,
													   									'AssetID'				=> self::$string_optional,
													   									'PaintLayoutType'		=> self::$string_non_optional,
													   									'AssetUrl'				=> self::$string_optional,
													   									'PaintLayoutCode'		=> self::$string_optional,
													   								   ),

														'ApplyAssetPaintLayoutResponse'	=> Array (
																							'ApplyAssetPaintLayoutResult'	=> self::$boolean_optional,
													   								   ),
													   ),
							'RemoveAssetPaintLayout'	=> Array (
														'RemoveAssetPaintLayout'	=> Array (
																						'PaintLayoutID'			=> self::$string_non_optional,
													   									'AssetID'				=> self::$string_optional,
													   									'PaintLayoutType'		=> self::$string_non_optional,
																						'AssetUrl'				=> self::$string_optional,
													   									'PaintLayoutCode'		=> self::$string_optional,
													   								   ),

														'RemoveAssetPaintLayoutResponse'	=> Array (
																						'RemoveAssetPaintLayoutResult'	=> self::$boolean_optional,
													   								   ),
													   ),

							'ApplyURLPaintLayout'	=> Array (
														'ApplyURLPaintLayout'		=> Array (

													   								   ),

														'ApplyURLPaintLayoutResponse'	=> Array (
																						'ApplyURLPaintLayoutResult'	=> self::$boolean_optional,
													   								   ),
													   ),
							'GetPaintLayoutFromURL'	=> Array (
														'GetPaintLayoutFromURL'			=> Array (
																						'URL'				=> self::$string_non_optional,
																						'PaintLayoutType'	=> self::$string_non_optional,
														   								'PaintLayoutCode'	=> self::$string_optional,
																					   ),
														'GetPaintLayoutFromURLResponse'	=> Array (
																							'AssetID' => self::$string_non_optional,
																						),
														),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;

	}//end getComplexTypes();


    /**
    * getComplexTypes: return all the complex types used by the API
    *
    * @return array
    * @access public
    */
    public static function getComplexTypes($func_list=Array())
    {
        $complex_types    = Array (
							'GetDesignFromURL'		=> Array (
                                           			     	'DesignLinkSummary'			=> Array (
																							'DesignID'			=> self::$string_non_optional,
                                           			       									'TypeCode'			=> self::$string_optional,
                                           			       								   ),
                                           			   ),
                          );

        $complex_types_available = parent::getComplexElements($complex_types);

        return $complex_types_available;

    }//end getComplexTypes()


	/**
	* getSimpleRestrictedTypes
	*
	* @return array
	* @access public
	*/
	public static function getSimpleRestrictedTypes($func_list=Array())
	{
		$design_type	= Array (
							'restriction_base'	=> 'string',
							'enumeration'		=> Array (
													'Frontend', 'OverrideFrontend', 'Login', 'OverrideLogin', 'UserDefined', 'OverrideUserDefined',
												   ),
						  );

		$simple_restricted_types	= Array (
							'ApplyDesign'		=> Array (
													'DesignType'	=> $design_type,
												   ),
							'RemoveDesign'		=> Array (
													'DesignType'	=> $design_type,
												   ),
							'GetDesignFromURL'	=> Array (
													'DesignType'	=> $design_type,
												   ),
						  );

		$simple_restricted_types_available = parent::getSimpleRestrictedTypes($simple_restricted_types);

		return $simple_restricted_types_available;

	}//end getSimpleRestrictedTypes()


}//end class
?>
