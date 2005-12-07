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
* $Id: tag_manager.inc,v 1.1 2005/12/07 22:55:23 emcdonald Exp $
*
*/


/**
* Tag Manager
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class Tag_Manager extends MySource_Object
{


	/**
	* Constructor
	*
	*/
	function Tag_Manager()
	{
		parent::MySource_Object();

	}//end constructor


	/**
	* Get all the tags on an asset
	*
	* @param int	$assetid	The asset who's tag we want.
	*
	* @return array
	* @access public
	*/
	function getTagLinks($assetid)
	{

		$tag_links = Array();
		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_NOTICE);
		foreach ($notice_links as $notice_link) {
			$id_parts = explode(':', $notice_link['minorid']);
			$thes_id = $id_parts[0];
			$thes = &$GLOBALS['SQ_SYSTEM']->am->getAsset($thes_id, 'thesaurus', true);
			if ($thes) $tag_links[] = $notice_link;
		}

		return $tag_links;

	}//end getTagLinks()


}//end class
?>