<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: folder.inc,v 1.39 2006/01/18 01:04:57 lwright Exp $
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
* @version $Revision: 1.39 $
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
		$this->Asset($assetid);

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
	function _preCreateCheck(&$link)
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
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// set a web path if we are linking this folder somewhere
		// NOTE: the main time we wont want to do this is if we are creating
		// the root folder as it is not linked anywhere
		if (!empty($link)) {
			// set an initial web path
			$initial_path = strtolower($this->attr('name'));
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_paths = make_valid_web_paths(Array($initial_path));
			$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
			if (!$this->saveWebPaths($good_paths)) return FALSE;
		}
		return TRUE;

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
	function _getAllowedLinks()
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
