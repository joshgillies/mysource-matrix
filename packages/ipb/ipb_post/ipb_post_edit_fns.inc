<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_post_edit_fns.inc,v 1.5 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/ipb/ipb_topic/ipb_topic_edit_fns.inc';

/**
* Ipb_Post_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Post_Edit_Fns extends Ipb_Topic_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ipb_Post_Edit_Fns()
	{
		$this->Ipb_Topic_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for the author of the post
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAuthor(&$asset, &$o, $prefix)
	{
		$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ipbid);
		$ipb =& $ipb_bridge->getIpbConn();
		$result = $ipb->getMemberInfoByName($asset->attr('author_name'));

		$o->openField(translate('Author'));

			echo get_asset_tag_line($asset->_ipbid.':'.'u_'.$result['id']);
		$o->closeField();

		return FALSE;

	}//end paintAuthor()


	/**
	* Paint the interface for the author of the post
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAuthor(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processAuthor()


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
	function paintPostContent(&$asset, &$o, $prefix)
	{
		$o->openField(translate('Post'));

			echo $asset->attr('post');
		$o->closeField();

		return FALSE;

	}//end paintPostContent()


	/**
	* Process the interface for the post details
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPostContent(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processPostContent()


}//end class

?>
