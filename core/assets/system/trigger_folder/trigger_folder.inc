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
* $Id: trigger_folder.inc,v 1.3 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';
/**
* Trigger_Folder
*
* Purpose
*	Organize triggers
*
* @author  Edison Wang <ewang@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Folder extends Folder
{


	


	/**
	* Constructor
	*
	* @param string	$id		The ID of this Trigger Folder
	*
	* @return void
	* @access public
	*/
	function Trigger_Folder($id=0)
	{
		if(!$id) {
			return;
		}
		// get trigger manager id
		$id_parts = explode(':', $id);
		$tmid = $id_parts[0];
		
 		$this->id = $id;
 		$this->name = $id;
 		$this->tmid = $tmid;
	}//end constructor

	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		
		$links = Array();

		$links[SQ_LINK_TYPE_2]['trigger'] = Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   );

		return $links;

	}//end _getAllowedLinks()

	

	
	/**
	* Determine whether this asset can have its last significant link removed
	*
	* This function is most commonly called when moving the asset to the trash.
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()
	
	
	
	
}//end class


?>
