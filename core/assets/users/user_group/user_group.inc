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
* $Id: user_group.inc,v 1.21 2006/01/26 22:34:07 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* User_Group
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User_Group extends Folder
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function User_Group($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['security']    = max($lock_types) * 2;
		$lock_types['preferences'] = max($lock_types) * 2;
		return $lock_types;

	}//end lockTypes()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_TYPE_1]['user_group'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['user']       = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['user_group'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['user']       = Array('card' => 'M', 'exclusive' => FALSE);
		return $allowed;

	}//end _getAllowedLinks()


	/**
	* Called by the design to print extra material in the head of the HTML document for this asset
	*
	* @return void
	* @access public
	*/
	function printHead()
	{
		?>
		<style type="text/css">
			.sq_user_group {
				width:				100%;
				border:				1px solid #CCCCCC;
				margin:				2px;
				padding:			0px;
			}
			.sq_user_group_heading, .sq_user_group_heading a:active, .sq_user_group_heading a:visited, .sq_user_group_heading a:hover {
				padding:			0px;
				font-weight:		bold;
				background-color:	#F0F0F0;
				font-size:			120%;
				text-decoration:	none;
			}
			.sq_user_group_heading a:hover {
				text-decoration:	underline;
			}
			.sq_user_group_display_name {
				font-weight:		bold;
			}
			.sq_user_group_user_name {
				background-color:	#F0F0F0;
				width:				200px;
			}
			.sq_user_group_user_email {
				width:				200px;
			}
			.sq_user_group_user_name, .sq_user_group_user_email, .sq_user_group_user_data {
				padding:			0px;
			}
		</style>
		<?php

	}//end printHead()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$cm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$cached_contents = $cm->loadFromCache($this->id, $this->type(), 'default');

		// if the cache is empty, go ahead and regenerate a new version
		if (empty($cached_contents)) {
			ob_start();
				$this->printUserList($this->id);
				$groups = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'user_group', FALSE);
				foreach ($groups as $groupid => $type_code) {
					$this->printUserList($groupid);
				}

				$cm->saveToCache($this->id, $this->type(), 'default', ob_get_contents());
			ob_end_flush();
		} else {
			echo $cached_contents;
		}

	}//end printBody()


	/**
	* Prints a list of users for the passed group
	*
	* @param string	$groupid	the ID of the group that we are printing the user list for
	*
	* @return void
	* @access public
	*/
	function printUserList($groupid)
	{
		$group =& $GLOBALS['SQ_SYSTEM']->am->getAsset($groupid);

		?>
		<table class="sq_user_group">
			<tr>
				<td class="sq_user_group_heading" colspan="3">
					<?php
					$url = $group->getURL();
					if (!empty($url)) echo '<a href="'.$url.'">';
					echo $group->name;
					if (!empty($url)) echo '</a>';
					?>
				</td>
			</tr>
			<?php
			$users = $GLOBALS['SQ_SYSTEM']->am->getLinks($groupid, SQ_LINK_TYPE_1, 'user', FALSE);
			if (!empty($users)) {
				?>
				<tr>
					<td class="sq_user_group_display_name">Name</td>
					<td class="sq_user_group_display_name">Email</td>
					<td class="sq_user_group_display_name">Username</td>
				</tr>
				<?php
				foreach ($users as $link_data) {
					$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid'], $link_data['minor_type_code']);
					?>
					<tr>
						<td class="sq_user_group_user_name"><?php echo $user->name; ?></td>
						<td class="sq_user_group_user_email">
							<a href="mailto:<?php echo $user->attr('email'); ?>"><?php echo $user->attr('email'); ?></a>
						</td>
						<td class="sq_user_group_user_data"><?php echo $user->attr('username'); ?></td>
					</tr>
					<?php
				}//end foreach

			}//end if there are users

		echo '</table>';

	}//end printUserList()


}//end class
?>
