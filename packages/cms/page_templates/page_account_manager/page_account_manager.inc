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
* $Id: page_account_manager.inc,v 1.12 2004/03/30 04:21:20 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page_templates/page_asset_builder/page_asset_builder.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Account_Manager
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Account_Manager extends Page_Asset_Builder
{

	/**
	* An array of keywords that this asset makes available for use along with a default replacement
	* @var Array(keyword => Array('name' => short description, 'default' => default replacement))
	*/
	var $_available_keywords = Array(
								'login_invite',
								'login_form',
								'login_error',
								'create_invite',
								'create_form',
								'create_error',
								'edit_details_invite',
								'edit_details_form',
								'edit_details_success',
								'edit_details_lock_error',
								'logout_form',
								);

	/**
	* An array of bodycopies that need to be created
	* @var Array()
	*/
	var $_bodycopies = Array( 'Not Logged In'    => 'not_logged_in',
							  'Created Not Live' => 'create_no_login',
							  'Created Live'     => 'create_login',
							  'Logged In'        => 'logged_in',
							 );


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Account_Manager($assetid=0)
	{
		$this->Page_Asset_Builder($assetid);

	}//end constructor


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		parent::printBody();

		$current_user = &$GLOBALS['SQ_SYSTEM']->user;
		if (!is_null($current_user) && !SQ_IN_BACKEND) {
			// this person is logged in - so redirect them to limbo where they can edit their details
			$redirect_url = strip_url(current_url()).'/'.SQ_CONF_LIMBO_SUFFIX.'/?ignore_frames=1';
			?>
			<script language="Javascript">
				document.location = "<?php echo $redirect_url; ?>";
			</script>
			Click <a href="<?php echo $redirect_url; ?>">here</a> to edit your details.
			<?php
		}

	}//end printBody()


	/**
	* Called by the design to print the body of this asset when in limbo
	*
	* Standard page puts its bodycopy up for editing instead of itself
	*
	* @return void
	* @access public
	*/
	function printLimbo()
	{	
		$GLOBALS['SQ_SYSTEM']->backend->out->addFormActionGetVar('ignore_frames', '1', true);

		$current_user = &$GLOBALS['SQ_SYSTEM']->user;
		$this->_current_state = 'logged_in';

		if (!is_null($current_user)) {
			// try and acquire a lock on the current user so they can edit their details
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $current_user->id, 'lock_type' => 'attributes', 'forceably_acquire' => false);
			$lock_errors = $hh->freestyleHipo('hipo_job_acquire_lock', $vars);


			  /////////////////////////
			 //  EDIT DETAILS FORM  //
			/////////////////////////

			ob_start();
				if (!empty($lock_errors)) {
					$this->_replacements['edit_details_lock_error'] = $this->attr('edit_details_lock_error');
					$current_user->printBody();
				} else {
					

					// ensure that the user has access to edit their details
					// ie. they have permission to access the default screen for a user
					require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';
					$aei = new Asset_Edit_Interface($current_user->type());

					$allowed_screens = $aei->getAllowedScreens($current_user);
					$default_screen = $aei->_default_screen;
					if (empty($default_screen)) $default_screen = 'details';

					$check_screen_restrictions = (!($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()));
					if ($check_screen_restrictions) if (isset($allowed_screens['0']['0'])) $check_screen_restrictions = false;
				
					if ($check_screen_restrictions && !isset($allowed_screens[$default_screen])) {
						$this->_replacements['edit_details_invite'] = '<p>You do not have access to edit your details.</p>';
						$current_user->printBody();
					} else {
						$this->_replacements['edit_details_invite'] = $this->attr('edit_details_invite');
						$current_user->printLimbo();
					}
					unset($aei);
				}
				$this->_replacements['edit_details_form'] = ob_get_contents();
			ob_end_clean();


			  ////////////////////
			 //  LOG OUT FORM  //
			////////////////////

			ob_start();
				?>
				<form name="account_manager_logout" method="POST" action="<?php echo current_url(true, true); ?>?SQ_ACTION=logout">
					<?php echo submit_button('logout', 'Logout'); ?>
				</form>
				<?php
				$this->_replacements['logout_form'] = ob_get_contents();
			ob_end_clean();
		}

		$this->_printContents();

	}//end printLimbo()


	/**
	* Process any global Account Manager actions
	*
	* This function builds on the Asset Builder one by auto filling the username field of
	* the login form that may be printed once the user account has been created.
	*
	* @return void
	* @access private
	* @see Page_Asset_Builder::_processGlobalActions()
	*/
	function _processGlobalActions()
	{
		parent::_processGlobalActions();

		if (isset($_POST['ASSET_BUILDER_ACTION'])) {
			switch (strtolower($_POST['ASSET_BUILDER_ACTION'])) {
				case 'create' :
					if (empty($this->_errors) && !empty($this->_tmp['created_asset'])) {
						// the new user was created and can log in, so make sure the page is in the right state
						if ($this->attr('create_status') >= SQ_STATUS_LIVE) {
							$this->_current_state = 'create_login';
							$_POST['SQ_LOGIN_USERNAME'] = $this->_tmp['created_asset']->attr('username');
							$this->_loadLoginReplacements();
						} else {
							$this->_current_state = 'create_no_login';
						}
					}
				break;
			}
		}

	}//end _processGlobalActions()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object Backend_Outputter	&$o
	* @param Array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		if (SQ_IN_LIMBO) {
			$current_user = &$GLOBALS['SQ_SYSTEM']->user;

			if (!is_null($current_user)) {
				if ($current_user->processBackend($o, $link)) {
					$this->_replacements['edit_details_success'] = $this->attr('edit_details_success');
				}
			}

			return true;
		}

		return parent::processBackend($o, $link);

	}//end processBackend()


	/**
	* Determine if the current user is allowed into this asset's backend interface
	*
	* @return boolean
	* @access public
	*/
	function backendAccess()
	{
		if (SQ_IN_LIMBO) return is_a($GLOBALS['SQ_SYSTEM']->user, 'user');
		else return parent::backendAccess();

	}//end backendAccess()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	&$broadcaster	the asset that triggered the event
	* @param Array			$vars			the vars that get submitted by the broadcaster
	*										we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		$keywords = Array();
		foreach ($this->_available_keywords as $keyword) {
			$name = ucwords(str_replace('_', ' ', $keyword));
			$type = 'inline';
			if ($keyword == 'logout_form') {
				$default = '<input style="border: 1px solid #5A5A5A; background-color: #F4F4EB;" type="button" value="Logout">';
				$name = 'Logout Button';
				$type = 'special';
			} else if (substr($keyword, -5) == '_form') {
				$default = $name;
				$type = 'block';
			} else {
				$default = $this->attr($keyword);
			}
			$keywords[$keyword] = Array('name' => $name, 'default' => $default, 'type' => $type);
		}
		$vars['keywords'] = array_merge($vars['keywords'], $keywords);
		
	}//end onRequestKeywords()


}//end class

?>
