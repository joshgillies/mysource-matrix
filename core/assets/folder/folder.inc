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
* $Id: folder.inc,v 1.43 2008/07/02 14:17:38 gnoel Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.43 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Folder extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Folder($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Folders ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Folders set an initial web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// NOTE: the main time we wont want to do this is if we are creating
		// the root folder as it is not linked anywhere
		if (empty($link)) return TRUE;

		return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name')), $link);

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
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
				SQ_LINK_TYPE_1	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_TYPE_3	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
			   );

	}//end _getAllowedLinks()


}//end class
?>
