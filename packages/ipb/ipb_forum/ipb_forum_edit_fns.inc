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
* $Id: ipb_forum_edit_fns.inc,v 1.5 2012/08/30 00:58:55 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Ipb_Forum_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Forum_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ipb_Forum_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens = Array(
									'details'		=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
									'dependants'	=> Array(
														'name'			=> 'Dependants',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
								);

	}//end constructor


	/**
	* Paint the interface for the list of recent posts
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRecentPosts(&$asset, &$o, $prefix)
	{
		$tmp = explode(':', $asset->id);
		$tmp = explode('_', $tmp[1]);

		$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ipbid);
		$ipb =& $ipb_bridge->getIpbConn();
		$posts = $ipb->getListOfPosts(0, $tmp[1], 10);

		if (empty($posts)) {
			$o->openField(translate('list_of_recent_posts'));
				echo translate('no_post_in_forum');
			$o->closeField();
		} else {
			$o->openField(translate('list_of_recent_posts'));
				?>
				<table class="sq-backend-table">
					<tr>
						<th style="width: 35%"><?php echo translate('topic'); ?></th>
						<th style="width: 45%"><?php echo translate('post'); ?></th>
						<th style="width: 20%"><?php echo translate('author'); ?></th>
					</tr>
				<?php
				foreach ($posts as $post) {
				?>
					<tr>
						<td><?php echo get_asset_tag_line($asset->_ipbid.':'.'t_'.$post['tid']); ?></td>
						<td><?php echo get_asset_tag_line($asset->_ipbid.':'.'p_'.$post['id']); ?></td>
						<td><?php echo get_asset_tag_line($asset->_ipbid.':'.'u_'.$post['author_id']); ?></td>
					</tr>
				<?php
				}
				echo '</table>';
			$o->closeField();
		}
		return FALSE;

	}//end paintRecentPosts()


	/**
	* Process the interface for the list of recent posts
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRecentPosts(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processRecentPosts()


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
				$author_tag_line = get_asset_tag_line($asset->_ipbid.':'.'u_'.$result['id']);
				echo $author_tag_line;
			$o->closeField();

		} else {

			$o->openField(translate('last_posted_by'));
				echo translate('none');
			$o->closeField();

		}

		return FALSE;

	}//end paintLastPosterName()


	/**
	* Paint the interface for the list of root forums
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintListOfRootForums(&$asset, &$o, $prefix)
	{
		$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ipbid);
		$ipb =& $ipb_bridge->getIpbConn();
		$root_forums = $ipb->getListOfRootForums();

		if (empty($root_forums)) {
			$o->openField(translate('list_of_root_forums'));
				echo translate('no_forums_in_ipb');
			$o->closeField();
		} else {
			$o->openField(translate('list_of_forums'));
				?>
				<table class="sq-backend-table">
					<tr>
						<th style="width: 50%"><?php echo translate('name'); ?></th>
						<th style="width: 50%"><?php echo translate('asset'); ?></th>
					</tr>
				<?php
				foreach ($root_forums as $root_forum) {
					$data = $ipb->getRootForumInfo($ipb_bridge->id.':'.'r_'.$root_forum['id']);
				?>
					<tr>
						<td style="font-weight: bold;"><?php echo $data['name']; ?></td>
						<td><?php echo  get_asset_tag_line($asset->_ipbid.':'.'r_'.$data['id']); ?></td>
					</tr>
				<?php
				}
			$o->closeField();
		}
		return FALSE;

	}//end paintListOfRootForums()


	/**
	* Paint the interface for the list of root forums
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processListOfRootForums(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processListOfRootForums()


	/**
	* Returns TRUE if this forum is used as a category
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return object
	* @access public
	*/
	function isCategory(&$asset, $prefix)
	{
		return $asset->_suffix == '0';

	}//end isCategory()


	/**
	* Returns TRUE if this forum is used as a normal forum
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return object
	* @access public
	*/
	function isNormalForum(&$asset, $prefix)
	{
		return (!$this->isCategory($asset, $prefix) && $asset->_prefix == 'f');

	}//end isNormalForum()


}//end class

?>
