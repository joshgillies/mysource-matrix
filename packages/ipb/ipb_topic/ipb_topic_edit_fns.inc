<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ipb_topic_edit_fns.inc,v 1.2 2005/12/21 04:18:25 skim Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/ipb/ipb_forum/ipb_forum_edit_fns.inc';

/**
* Ipb_Topic_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Topic_Edit_Fns extends Ipb_Forum_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ipb_Topic_Edit_Fns()
	{
		$this->Ipb_Forum_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for the post details
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintStarterName(&$asset, &$o, $prefix)
	{
		$starter_name = $asset->attr('starter_name');
		if (!empty($starter_name)) {

			$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ipbid);
			$ipb =& $ipb_bridge->getIpbConn();
			$result = $ipb->getMemberInfoByName($starter_name);

			$o->openField(translate('topic_started_by'));
				echo get_asset_tag_line($asset->_ipbid.':'.'u_'.$result['id']);
			$o->closeField();

		} else {

			$o->openField(translate('topic_started_by'));
				echo translate('none');
			$o->closeField();

		}

		return FALSE;

	}//end paintStarterName()


	/**
	* Paint the interface for the post details
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLastPosterName(&$asset, &$o, $prefix)
	{
		$last_poster_name = $asset->attr('last_poster_name');
		if (!empty($last_poster_name)) {

			$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ipbid);
			$ipb =& $ipb_bridge->getIpbConn();
			$result = $ipb->getMemberInfoByName($asset->attr('last_poster_name'));

			$o->openField(translate('last_posted_by'));
				echo get_asset_tag_line($asset->_ipbid.':'.'u_'.$result['id']);
			$o->closeField();

		} else {

			$o->openField(translate('last_posted_by'));
				echo translate('none');
			$o->closeField();

		}

		return FALSE;

	}//end paintLastPosterName()


}//end class

?>
