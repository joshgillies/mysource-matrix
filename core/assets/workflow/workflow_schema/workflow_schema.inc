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
* $Id: workflow_schema.inc,v 1.22 2012/12/07 05:33:46 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Workflow_Schema
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Schema extends Folder
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
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array (
	*   'asset'        => [ref major asset to create link under],
	*   'link_type'    => SQ_LINK_?,
	*   'value'        => [link value],
	*   'sort_order'   => [link sort order],
	*   'is_dependant' => [0|1],
	*   'is_exclusive' => [0|1],
	* )
	* </pre>
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		$ok = TRUE;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('workflow_stream');
		$new_stream = new Workflow_Stream();
		$new_stream->setAttrValue('name', 'Default Stream');

		$link_info = Array(
						'asset'			=> $this,
						'link_type'		=> SQ_LINK_TYPE_2,
						'value'			=> 'default_stream',
						'is_exclusive'	=> TRUE,
						'is_dependant'	=> FALSE,
					 );

		if (!$new_stream->create($link_info)) {
			$ok = FALSE;
			break;
		}

		return $ok;

	}//end _createAdditional()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	public function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


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
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(
									'workflow_stream'	=> Array(
															'card'		=> 'M',
															'exclusive'	=> FALSE,
														   ),
									'workflow_step'		=> Array(
															'card'		=> 'M',
															'exclusive'	=> FALSE,
														   ),
								   ),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param Asset	&$asset			the major or minor asset that we are linking to
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
		// if this is a workflow step then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Workflow_Stream) && $dependant != '1') {
			$dependant = '1';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


	/**
	* Attempts to override all current paths for this asset with those passed in the array
	*
	* @param array	$paths	array of paths to give this asset
	*
	* @return boolean
	* @access public
	*/
	public function saveWebPaths(Array $paths)
	{
		return TRUE;

	}//end saveWebPaths()


	/**
	* Delete this asset from the trash
	*
	* Note that the asset <i>MUST</i> be in the trash to delete it <i>FROM</i> the trash.
	*
	* @param boolean	$release_lock	should we realease the lock after deleting
	* @param boolean	$check_locked	Check if the link is locked or not
	*
	* @return boolean
	* @access public
	* @see asset:delete()
	*/
	public function delete($release_lock=TRUE, $check_locked=TRUE)
	{
		// lets have with us all the assets the schema we are deleteing is applied to
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schema_treeids_sql = 'SELECT
								assetid, granted, wflow
								FROM (sq_ast_lnk_tree ta
									JOIN sq_ast_lnk la ON ta.linkid = la.linkid
										JOIN sq_ast_wflow wa ON la.minorid = wa.assetid)
								WHERE wa.schemaid = :schemaid
								AND NOT EXISTS (
									SELECT * FROM (sq_ast_lnk_tree tb
													JOIN sq_ast_lnk lb ON tb.linkid = lb.linkid
														JOIN sq_ast_wflow wb ON lb.minorid = wb.assetid)
									WHERE wb.schemaid = wa.schemaid
									AND length(ta.treeid) - LENGTH(tb.treeid) = :tree_size 
									AND SUBSTR(ta.treeid, 1, LENGTH(ta.treeid) - :tree_size_1) = tb.treeid
									AND wa.granted = wb.granted
									AND wa.cascades = wb.cascades
								) ORDER BY assetid';

		$query = MatrixDAL::preparePdoQuery($schema_treeids_sql);
		MatrixDAL::bindValueToPdo($query, 'schemaid', $this->id);
		MatrixDAL::bindValueToPdo($query, 'tree_size', SQ_CONF_ASSET_TREE_SIZE);
		MatrixDAL::bindValueToPdo($query, 'tree_size_1', SQ_CONF_ASSET_TREE_SIZE);
		$results = MatrixDAL::executePdoAssoc($query);

		// check for running schema
		$running_schema = FALSE;
		$running_id = array();
		foreach ($results as $index => $info) {
		    if(!empty($info['wflow'])) {
			$running_schema = TRUE;
			$running_id[] = $info['assetid'];
		    }
		}
		if($running_schema) {
		    trigger_error('Purged Workflow Schema Id #'.$this->id.' is applied and running on assets with Id: '.  implode(', ', $running_id), E_USER_NOTICE);
		    return FALSE;
		}
		
		// guess what? someone is deleting schema even when it might be applied to assets.
		// lets be smart and remove from the assets where schema is NOT running
		foreach ($results as $index => $info) {
			if(empty($info['wflow'])) {
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$wfm->deleteSchema($info['assetid'], $this->id);
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			} else {
				// oops someone is deleting the workflow without considering that it might
				// be running on some assets.
				trigger_error('Purged Workflow Schema Id #'.$this->id.' is applied and running on Asset Id #'.$info['assetid'].'.', E_USER_NOTICE);
				return FALSE;
			}
		}

		// we have now removed the db entries removed...all set to delete the schema
		if(!parent::delete($release_lock, $check_locked)) {
			return FALSE;
		}

		return TRUE;

	}//end delete()


	/**
	* If the asset supports System Version mode (Safe Edit status)
	*
	* @return boolean
	* @access public
	*/
	function supportsSystemVersion()
	{
		return FALSE;

	}//end supportsSystemVersion()


}//end class

?>
