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
* $Id: inbox.inc,v 1.37 2013/08/19 01:12:24 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Inbox
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.37 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Inbox extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Inbox($assetid=0)
	{
		$this->Asset($assetid);

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
		$lock_types['mail'] = max($lock_types) * 2;
		return $lock_types;

	}//end lockTypes()


	/**
	* Gets the user that this inbox is attached to
	*
	* @return object
	* @access private
	*/
	function _getUser()
	{
		$user = NULL;

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'user', FALSE, '', 'minor', '1');
		if (!empty($link)) {
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
		}

		return $user;

	}//end _getUser()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Returns TRUE if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	*
	* @param int	$perm		the permission to check access for
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function _checkPermissionAccess($perm, $assetids=Array())
	{
		$user = $this->_getUser();
		if (is_null($user)) {
			return parent::_checkPermissionAccess($perm, $assetids);
		}

		return $user->_checkPermissionAccess($perm, $assetids);

	}//end _checkPermissionAccess()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		$this->printBody();

	}//end printFrontend()


	/**
	* Prints out the backend interface for the asset
	* it's up to the kids to override
	*
	* @param object	$o	the Backend Outputter object
	*
	* @return void
	* @access public
	*/
	function paintBackend($o)
	{
		if (isset($_REQUEST['gettrash'])) {
			$user = $this->_getUser();

			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$messages = $ms->getMessages($user->id, NULL, Array(SQ_MSG_DELETED), Array(), NULL, NULL, 'name');
			echo '<root>';
			foreach ($messages as $key => $message) {
				echo '<message from="'.$message['from_name'].'" id="'.$message['msgid'].'" date="'.$GLOBALS['SQ_SYSTEM']->datetime($message['sent']).'"';
				echo ' subject="'.$message['subject'].'">'.$message['body'].'</message>';
			}
			echo '</root>';
			exit();
		}

		parent::paintBackend($o);

	}//end paintBackend()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		// they want to print all mail
		if (!SQ_IN_BACKEND) {
			$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'mail');
			$user = $this->_getUser();
			?>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/general.js"></script>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/html_form/html_form.js"></script>
			<h2 style="margin-bottom: 2px">Inbox for <?php echo $user->attr('first_name').' '.$user->attr('last_name'); ?></h2>
			<form method="POST" action="<?php echo htmlentities($_SERVER['REQUEST_URI'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>" name="main_form" id="main_form">
			<?php
		}

		$null = '';
		$edit = $this->getEditFns();
		$edit->paintInbox($this, $null, 'inbox');

		if (!SQ_IN_BACKEND) {
			?>
			</form>
			<?php
		}

	}//end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return 'Inbox';

	}//end _getName()


	/**
	* Returns an attribute of the asset
	*
	* @param string	$name	The name of the attribute to return
	*
	* @return string
	* @access public
	*/
	public function attr($name)
	{
		if ($name == 'name') {
			return $this->_getName();
		} else {
			return parent::attr($name);
		}//end if

	}//end attr()


}//end class

?>
