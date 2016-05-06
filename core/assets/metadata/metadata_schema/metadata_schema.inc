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
* $Id: metadata_schema.inc,v 1.29 2012/08/30 01:09:09 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Metadata_Schema
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Schema extends Folder
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
				SQ_LINK_TYPE_2	=> Array('metadata_section'	=> Array('card'	=> 'M', 'exclusive'	=> FALSE)),
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
		// if this is a metadata section then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Metadata_Section) && $dependant != '1') {
			$link_type = SQ_LINK_TYPE_2;
			$dependant = '1';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


	/**
	* Delete this asset from the trash
	*
	* Removes any reference to this schema from the 'sq_ast_mdata' table
	*
	* @param boolean	$release_lock	should we realease the lock after deleting
	* @param boolean    $check_locked   Check if the link is locked or not
	*
	* @return boolean
	* @access public
	*/
	public function delete($release_lock=TRUE, $check_locked=TRUE)
	{
		if (parent::delete($release_lock)) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db = MatrixDAL::getDb();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// First remove any values assets have for this field
			try {
				$sql = 'DELETE FROM sq_ast_mdata WHERE ';
				$sql .= 'schemaid = :schemaid';

				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'schemaid', $this->id);

				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to delete metadata schema with schemaid #'.$this->id.' due to the following database error:'.$e->getMessage());
			}//end try catch

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			return TRUE;
		}
		return FALSE;

	}//end delete()


	/**
	* Attempts to override all current paths for this asset with those passed in the array
	*
	* @param array	$paths	array of paths to give this asset
	* @param boolean    $auto_add_remaps    whether or not to add remaps for the asset when updating lookups
	*
	* @return boolean
	* @access public
	*/
	public function saveWebPaths($paths, $auto_add_remaps = TRUE)
	{
		return TRUE;

	}//end saveWebPaths()


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
