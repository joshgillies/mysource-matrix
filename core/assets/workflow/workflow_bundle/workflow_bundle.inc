<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: workflow_bundle.inc,v 1.10 2012/08/30 01:09:17 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';

/**
* Workflow_Bundle
*
* Purpose
*   Extends from Bridge, but only to allow NOTICE links to be shown in the
*   asset map. There are no shadow assets exposed by this bridge asset.
*
*   Workflow Bundles have their own status system; statuses are found in the
*   asset_status/ sub-directory of the asset.
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Bundle extends Asset implements Bridge
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns the name of the asset
	*
	* @param boolean	$short_name	TRUE = return short name; FALSE = return full name
	*
	* @return string
	* @access public
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_NOTICE => Array(
									'asset' => Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								  ),
			   );

	}//end _getAllowedLinks()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* Nothing doing here. We aren't providing shadow assets to y'all, so go away.
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	public function getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		return NULL;

	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	public function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
		return Array();

	}//end getLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int		$assetid			the id of the asset to get its parents for
	* @param mixed		$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*										if an array returns link if matches any of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	public function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
		return Array();

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* Returns an array of assetids and their type code
	*
	* I believe that this relates to structural links underneath us (cf. Search
	* Folder). So this should be empty no matter what, since we only have NOTICE
	* links.
	*
	* @param int		$assetid			the id of the asset to get its children for
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param boolean	$dependant			if TRUE, results will be filtered to assets that are dependants.
	*										If FALSE, results will all not be dependants.
	*										If NULL, results will not be filtered
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	public function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		return Array();

	}//end getChildren()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* No URLs to propagate here, since we have no structural links that we need
	* to provide lookups for.
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> NULL means it is ignored
	* @param string	$url		the url to check for -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	public function getLineageFromURL($assetid, $protocol, $url)
	{
		return Array();

	}//end getLineageFromURL()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type and/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc) (can be a string or an array of strings)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link id when returning the count
	*
	* @return int
	* @access public
	*/
	public function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		return 0;

	}//end countLinks()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	public function deleteAssetLink($linkid, $moving=FALSE)
	{
		return FALSE;

	}//end deleteAssetLink()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* We need to implement this so our NOTICE links are shown in the asset
	* map.
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$links_query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($this->id, SQ_LINK_NOTICE, 'asset', FALSE);
		if (empty($links_query)) return Array();

		$db_links = Array();

		try {
			$query = MatrixDAL::preparePdoQuery(implode(' ', $links_query['sql_array']));
			foreach ($links_query['bind_vars'] as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$db_links = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get links for asset: '.$assetid.' due to database error: '.$e->getMessage());
		}

		// Grab the assets from the links query so we can get their info all at once
		$bundled_assets = Array();
		$linkids = Array();
		foreach ($db_links as $db_link) {
			$bundled_assets[] = $db_link['minorid'];
			$linkids[$db_link['minorid']] = $db_link['linkid'];
		}

		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($bundled_assets);
		$sort_order = 1;

		$links = Array();
		foreach ($asset_info as $bundled_assetid => $info) {
			$links[] = Array(
						'majorid'			=> $this->id,
						'minorid'			=> $bundled_assetid,
						'assetid'			=> $bundled_assetid,
						'type_code'			=> $info['type_code'],
						'linkid'			=> $linkids[$bundled_assetid],

						'name'				=> $info['name'],
						'short_name'		=> $info['short_name'],
						'status'			=> $info['status'],
						'link_type'			=> SQ_LINK_TYPE_2,
						'sort_order'		=> $sort_order,

						'url'				=> '',
						'path'				=> '',
						'num_kids'			=> -1,
						'accessible'		=> 1,
						'is_dependant'		=> 0,
					 );

			$sort_order++;
		}

		return $links;

	}//end getAssetMapLinks()


	/**
	* Get asset info for use by asset map
	*
	* Should return an array containing the following elements:<ul>
	* 	<li>assetid:    ID of the asset.</li>
	* 	<li>name:       Full name of the asset.</li>
	*	<li>short_name: Short name of the asset.</li>
	*	<li>version:    Asset version number.<li>
	*	<li>status:     Current status of the asset.</li>
	*	<li>type_code:  The type code of the shadow asset.</li>
	*   <li>num_kids:   Number of children, if known.</li>
	*	<li>accessible: Whether the asset should be accessible.</li>
	*	<li>url:        URL for this lineage of the asset, if known.</li>
	*	<li>web_path:   Web path for this lineage of the asset, if known.</li>
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	public function getAssetMapAssetInfo($assetid)
	{
		$map_info = Array();
		if ((string)$assetid === (string)$this->id) {
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);
			foreach ($links as $link) {
				$linked_assetid = $links['minorid'];
				$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($linked_assetid);
				$map_info[] = Array(
									'assetid'		=> $linked_assetid,
									'name'			=> $asset_info[$linked_assetid]['name'],
									'short_name'	=> $asset_info[$linked_assetid]['short_name'],
									'version'		=> $asset_info[$linked_assetid]['version'],
									'status'		=> $asset_info[$linked_assetid]['status'],
									'type_code'		=> $asset_info[$linked_assetid]['type_code'],
									'num_kids'		=> NULL,
									'accessible'	=> TRUE,
									'url'			=> NULL,
									'web_path'		=> NULL,
							  );
			}
		}

		return $map_info;

	}//end getAssetMapAssetInfo()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param Asset	$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	public function prepareLink(Asset $asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		$changed = FALSE;

		if ($side_of_link === 'major') {
			$link_type = SQ_LINK_NOTICE;
			$changed = TRUE;
		}

		return $changed;

	}//end prepareLink()


	/**
	* Returns a status object for the current asset to be used for processing
	*
	* This is generally the same as the method in Asset itself, bar the subtle
	* difference of it getting one of the Bundle status objects instead.
	*
	* @return Asset_Status
	* @access public
	*/
	public function getStatus()
	{
		if (!isset($this->_tmp['status_object'])) {
			// work out the name of our status file
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$status_code = get_bit_names('SQ_STATUS_', $this->status);
			$status_code = 'asset_status_bundle_'.strtolower($status_code);
			require_once dirname(__FILE__).'/asset_status/'.$status_code.'.inc';
			$this->_tmp['status_object'] = new $status_code($this);
		}
		return $this->_tmp['status_object'];

	}//end getStatus()


	/**
	* Event listener for bundled assets cancelling their workflow
	*
	* If any asset is rejected, any other bundled asset in workflow should be
	* rolled back, as well as the bundle itself.
	*
	* Note: status changes to the bundle are done with status integrity turned
	* off because a normal user could not actually do these changes. Once the
	* last asset with workflow is completed, the user then has no access to
	* status changes.
	*
	* @param Asset	$broadcaster	The broadcasting asset
	* @param array	$event_vars		Any event variables sent by the broadcaster
	*
	* @return void
	*/
	public function onAssetStatusUpdate(Asset $broadcaster, $event_vars)
	{
		$old_status = (int)$event_vars['old_status'];
		$new_status = (int)$event_vars['new_status'];

		if ((($old_status & SQ_SC_STATUS_PENDING) !== 0) && (($new_status & SQ_SC_STATUS_ALL_APPROVED) !== 0)) {
			if ((int)$this->status !== SQ_STATUS_PENDING_APPROVAL) return;

			$hipo_herder = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			if (array_key_exists('completing_workflow', $this->_tmp) === FALSE) {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

				$links_query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($this->id, SQ_LINK_NOTICE, 'asset', FALSE);
				if (empty($links_query)) return Array();

				// Modify the links query so that it only selects bundled assets
				// in Pending Approval or Safe Edit Pending Approval statuses
				$links_query['sql_array']['where'] .= ' AND minorid IN (SELECT assetid FROM sq_ast WHERE '.db_extras_bitand(MatrixDAL::getDbType(), 'status', ':all_status').' > 0)';
				$links_query['bind_vars']['all_status'] = SQ_SC_STATUS_PENDING;

				try {
					$query = MatrixDAL::preparePdoQuery(implode(' ', $links_query['sql_array']));
					foreach ($links_query['bind_vars'] as $bind_var => $bind_value) {
						MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
					}
					$db_links = MatrixDAL::executePdoAssoc($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get links for asset: '.$assetid.' due to database error: '.$e->getMessage());
				}

				// If we have no assets in Pending Approval statuses, there is
				// nothing left with a workflow. Proceed.
				if (count($db_links) === 0) {
					$this->_tmp['completing_workflow'] = 1;

					$run_level = $GLOBALS['SQ_SYSTEM']->getRunLevel();
					$GLOBALS['SQ_SYSTEM']->setRunLevel($run_level ^ SQ_SECURITY_STATUS_INTEGRITY);
					$vars = Array(
								'assetid'			=> $this->id,
								'new_status'		=> SQ_STATUS_APPROVED,
								'dependants_only'	=> TRUE,
							);
					$errors = $hipo_herder->freestyleHipo('hipo_job_edit_status', $vars);
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}

			}

		} else if ((($old_status & (SQ_SC_STATUS_PENDING | SQ_SC_STATUS_ALL_APPROVED)) !== 0) && (($new_status & SQ_SC_STATUS_CAN_APPROVE) !== 0)) {

			if ((($this->status & (SQ_STATUS_PENDING_APPROVAL | SQ_STATUS_APPROVED)) === 0)) {
				return;
			}

			$hipo_herder = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			$run_level = $GLOBALS['SQ_SYSTEM']->getRunLevel();
			$GLOBALS['SQ_SYSTEM']->setRunLevel($run_level ^ SQ_SECURITY_STATUS_INTEGRITY);
			$vars = Array(
						'bundle_assetid'	=> $this->id,
						'new_status'		=> SQ_STATUS_UNDER_CONSTRUCTION,
					);
			$hipo_herder->freestyleHipo('hipo_job_edit_bundled_assets_status', $vars);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		} else if (((int)$new_status === SQ_STATUS_LIVE) && ((int)$old_status !== SQ_STATUS_EDITING)) {
			// Transition to Live (but not cancelling a Safe Edit)
			// Cannot transition from Approving straight to Live...we can go from
			// Approved, or without workflow we can go from Editing
			if ((int)$this->status === SQ_STATUS_PENDING_APPROVAL) return;

			$hipo_herder = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			if (array_key_exists('completing_workflow', $this->_tmp) === FALSE) {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

				$links_query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($this->id, SQ_LINK_NOTICE, 'asset', FALSE);
				if (empty($links_query)) return Array();

				// Modify the links query so that it only selects bundled assets
				// NOT in Live status
				$links_query['sql_array']['where'] .= ' AND minorid IN (SELECT assetid FROM sq_ast WHERE '.db_extras_bitand(MatrixDAL::getDbType(), 'status', ':all_status').' = 0)';
				$links_query['bind_vars']['all_status'] = SQ_STATUS_LIVE;

				try {
					$query = MatrixDAL::preparePdoQuery(implode(' ', $links_query['sql_array']));
					foreach ($links_query['bind_vars'] as $bind_var => $bind_value) {
						MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
					}
					$db_links = MatrixDAL::executePdoAssoc($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get links for asset: '.$assetid.' due to database error: '.$e->getMessage());
				}

				// If we have no assets NOT in Live status, so we should go live
				if (count($db_links) === 0) {
					$run_level = $GLOBALS['SQ_SYSTEM']->getRunLevel();
					$GLOBALS['SQ_SYSTEM']->setRunLevel($run_level ^ SQ_SECURITY_STATUS_INTEGRITY);
					$vars = Array(
								'assetid'			=> $this->id,
								'new_status'		=> SQ_STATUS_LIVE,
								'dependants_only'	=> TRUE,
							);
					$hipo_herder->freestyleHipo('hipo_job_edit_status', $vars);
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}

			}

		}

		return;

	}//end onAssetStatusUpdate()


    /**
    * Returns TRUE if at least one of the passed assetids has read access to this asset
    *
    * @param array  $assetids   An array of user_group and/or user ids to check for read access
    *                           If the array is empty, the current user will be checked
    *
    * @return boolean
    * @access public
    */
    public function readAccess(Array $assetids=Array())
    {
        if (!$this->id) return TRUE;
        if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
            return TRUE;
        }

        /*
        * We need to provide Read Permission to the Bundle to ensure that they
        * can view Bundle details and advance Workflow if available.
        */
        $edit_fns = $this->getEditFns();
        if ($edit_fns->showIfCanApprove($this)) {
            return TRUE;
        }

        return $this->_checkPermissionAccess(SQ_PERMISSION_READ, $assetids);

    }//end readAccess()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* @param Asset  $minor      the minor asset that we are linking to
	* @param string $link_type  the type of link this is
	* @param int    $exclusive  the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed boolean|string
	* @access public
	*/
	public function canCreateLink(Asset $minor, $link_type, $exclusive)
	{
		if (!$this->id) return translate('Asset not created');


		// Ensure that Shadow Assets cannot be linked to a Bundle
		$minor_asset_id_parts = explode(':', $minor->id);
		if (isset($minor_asset_id_parts[1])) {
			return FALSE;
		}

		return parent::canCreateLink($minor, $link_type, $exclusive);

	}//end canCreateLink()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param Asset      $minor      the minor asset that we are linking to
	* @param string     $link_type  the type of link this is
	* @param string     $value      the value that is to be associated with this link
	* @param string     $sort_order the position in the links list that this link should take,
	*                               if NULL or less than zero places at end of list
	* @param string     $dependant  '0' / '1' on whether the this asset is dependant on the
	*                               asset that will be linked by this new link
	* @param string     $exclusive  '0' / '1' on whether the this asset is linked exclusivly
	*                               to the asset that will be linked by this new link
	* @param boolean    $moving     moving
	*
	* @return int
	* @access public
	* @see Asset_Manager::createAssetLink()
	*/
	public function createLink(Asset $minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE)
	{
		if (!$this->id) return 0;

		// Ensure that Shadow Assets cannot be linked to a Bundle
		$minor_asset_id_parts = explode(':', $minor->id);
		if (isset($minor_asset_id_parts[1])) {
			return 0;
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive, $moving);

	}//end createLink()


	/**
	* Returns all available workflow streams in bundled assets, EXCEPT default streams
	*
	* Array is of (stream_assetid => stream name)
	*
	* @return boolean
	*/
	public function getAllBundleStreams()
	{
		$bundled_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

		$all_schemas = Array();
		foreach ($bundled_asset_links as $linkid => $bundled_asset_link) {
			$bundled_assetid = $bundled_asset_link['minorid'];
			$bundled_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($bundled_assetid);

			if ($bundled_asset->adminAccess() === TRUE) {
				$all_schemas = array_merge($all_schemas, $wfm->getSchemas($bundled_assetid, TRUE));
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bundled_asset);
			unset($bundled_asset);
		}

		$all_schemas = array_unique($all_schemas);
		$all_streams = Array();

		foreach ($all_schemas as $schemaid) {
			$all_streams += $wfm->getStreams($schemaid, TRUE);
		}

		return $all_streams;

	}//end getAllBundleStreams()


	/**
	* Returns all available workflow streams in bundled assets that would be
	* bypassable in ALL of our bundled assets
	*
	* @return array
	*/
	public function getAllBypassableBundleStreams()
	{
		$bundled_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

		$all_streams = array_keys($this->getAllBundleStreams());
		foreach ($bundled_asset_links as $linkid => $bundled_asset_link) {
			$bundled_assetid = $bundled_asset_link['minorid'];
			$schemas = $wfm->getSchemas($bundled_assetid, TRUE);
			foreach ($schemas as $schemaid) {
				$all_streams = array_intersect($all_streams, $wfm->getBypassableStreams($bundled_assetid, $schemaid));
			}
		}

		return $all_streams;

	}//end getAllBypassableBundleStreams()


	/**
	* Is the default stream of all bundled assets bypassable?
	*
	* @return boolean
	*/
	public function isDefaultStreamBypassable()
	{
		$bundled_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

		foreach ($bundled_asset_links as $linkid => $bundled_asset_link) {
			$bundled_assetid = $bundled_asset_link['minorid'];
			if ($wfm->testPublish($bundled_assetid, $GLOBALS['SQ_SYSTEM']->user->id, '') === FALSE) {
				return FALSE;
			}
		}

		return TRUE;

	}//end isDefaultStreamBypassable()


}//end class

?>
