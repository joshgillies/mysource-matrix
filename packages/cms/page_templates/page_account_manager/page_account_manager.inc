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
* $Id: page_account_manager.inc,v 1.1 2003/11/27 02:48:47 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page.inc';
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
class Page_Account_Manager extends Page
{

	/**
	* The current state the account manager is in
	* Used for determining which bodycopy to use for printing
	* @var string
	*/
	var $_current_state = '';

	/**
	* An array of errors reported during execution
	* @var Array()
	*/
	var $_errors = Array();

	/**
	* An array of keyword replacements to make
	* @var Array()
	*/
	var $_replacements = Array();


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Account_Manager($assetid=0)
	{
		$this->Page($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1, 'value' => '');

		$bodycopies = Array('Not Logged In'    => 'not_logged_in',
							'Created Not Live' => 'create_no_login',
							'Created Live'     => 'create_login',
							'Logged In'        => 'logged_in',
							);
		foreach ($bodycopies as $name => $value) {
			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $name);
			$copy_link['value'] = $value;
			if (!$bodycopy->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	*
	* @return int linkid of the created link
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a bodycopy table then we need to make it a dependant link
		if (is_a($minor, 'bodycopy')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}//end createLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the bodycopy and the account manager page';

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 4, 'exclusive' => true);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$current_user = &$GLOBALS['SQ_SYSTEM']->user;
		$this->_current_state = 'not_logged_in';

		// process some global account manager actions
		if (isset($_POST['ACCOUNT_MANAGER_ACTION'])) {

			switch (strtolower($_POST['ACCOUNT_MANAGER_ACTION'])) {

				case 'login' :
					// they have tried logging in - but failed
					$this->_replacements['login_error'] = $this->attr('login_error');
				break;

				case 'create' :

					// we'll just use a limbo outputter to do the outputting for us
					require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
					$o = new Limbo_Outputter();

					$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('create_groupid'));
					if (is_null($parent)) {
						$this->_replacements['create_error'] = $this->attr('create_error');
						break;
					}

					$create_type = $this->attr('create_type');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
					$new_user = new $create_type();

					// start the transaction to create the user
					$link = Array('asset'		=> &$parent,
								  'link_type'	=> SQ_LINK_TYPE_2,
								  'value'		=> '',
								  'sort_order'	=> '');

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					// set the install flag because we are creating an asset without being
					// logged in - so any write access checks will fail
					$GLOBALS['SQ_INSTALL'] = true;
						set_error_handler(Array(&$this, '_errorHandler'));
						if (!$new_user->processBackend($o, $link)) {

							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							restore_error_handler();

							// create a friendly error message
							$errors = '';
							foreach ($this->_errors as $text) $errors .= '<li>'.$text.'</li>';
							$this->_replacements['create_error'] = $this->attr('create_error').'<ul>'.$errors.'</ul>';

						} else {

							$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
							restore_error_handler();

							$attr = $this->getAttribute('create_options');
							$create_options = $attr->_str2ar($attr->value);

							$this->_current_state = 'create_no_login';

							if (in_array('create_live', $create_options)) {
								// make this new user LIVE so they can log in
								$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
								$vars = Array('assetid' => $new_user->id, 'new_status' => SQ_STATUS_LIVE);
								$status_errors = $hh->freestlyeHipo('hipo_job_edit_status', $vars);
								if (empty($status_errors)) {
									$this->_current_state = 'create_login';
									$_POST['SQ_LOGIN_USERNAME'] = $new_user->attr('username');
								}
							}

						}//end if processBackend
					$GLOBALS['SQ_INSTALL'] = false;

				break;

			}//end switch

		}//end if ACCOUNT_MANAGER_ACTION


		if (!is_null($current_user)) {

			// this person is logged in - so redirect them to limbo where they can edit their details
			$redirect_url = strip_url(current_url()).'/'.SQ_CONF_LIMBO_SUFFIX.'/?ignore_frames=1';
			?>
			<script language="Javascript">
				document.location = "<?php echo $redirect_url; ?>";
			</script>
			Click <a href="<?php echo $redirect_url; ?>">here</a> to edit your details.
			<?php

		} else {

			// this person is not logged in, so we should let them log in or sign up for an account

			  ///////////////////
			 //  LOG IN FORM  //
			///////////////////

			$this->_replacements['login_invite'] = $this->attr('login_invite');

			ob_start();
				?>
				<form name="account_manager_login" method="POST" action="<?php echo $_SERVER['PHP_SELF']; ?>?SQ_ACTION=login" onSubmit="Javascript: return (this.elements['SQ_LOGIN_USERNAME'].value.length > 0 && this.elements['SQ_LOGIN_PASSWORD'].value.length > 1);">
					<input type="hidden" name="SQ_LOGIN_KEY"      value="<?php echo $GLOBALS['SQ_SYSTEM']->generateLoginKey(); ?>">
					<input type="hidden" name="SQ_LOGIN_REFERER"  value="">

					<table border="0">
						<tr>
							<td valign="top">Username</td>
							<td valign="top"><?php echo text_box('SQ_LOGIN_USERNAME', (isset($_POST['SQ_LOGIN_USERNAME']) ? $_POST['SQ_LOGIN_USERNAME'] : 'Enter your username here'), 25, 0, (isset($_POST['SQ_LOGIN_USERNAME']) ? false : true), 'title="Username"'); ?></td>
						</tr>
						<tr>
							<td valign="top">Password</td>
							<td valign="top"><?php echo password_box('SQ_LOGIN_PASSWORD', '', 25, 0, 'title="Password"'); ?></td>
						</tr>
						<tr>
							<td valign="top">&nbsp;</td>
							<td valign="top"><?php echo submit_button('ACCOUNT_MANAGER_ACTION', 'Login'); ?></td>
						</tr>
					</table>
				</form>
				<?php
				$this->_replacements['login_form'] = ob_get_contents();
			ob_end_clean();


			  ////////////////////
			 //  SIGN UP FORM  //
			////////////////////

			$parent_assetid = $this->attr('create_groupid');
			if (!$parent_assetid) {

				$this->_replacements['create_form'] = 'NO GROUPID SET. CANNOT CREATE A NEW USER';

			} else {

				ob_start();
					$this->_replacements['create_invite'] = $this->attr('create_invite');

					// close you eyes while you go through this next bit of code
					// its not pretty and tries to fudge some stuff so we can pretend
					// we are in the backend when we are not even in limbo

					require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
					$o = new Limbo_Outputter();

					// print the create interface for the asset
					$create_type = $this->attr('create_type');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
					$asset = new $create_type();
					$asset->paintBackend($o);

					?>
					<form name="account_manager_login" method="POST" action="<?php echo $_SERVER['PHP_SELF']; ?>">
						<input type="hidden" name="asset_action" value="create">
						<input type="hidden" name="ACCOUNT_MANAGER_ACTION"  value="create">
						<?php
							// paint the buffered sections
							$o->paintSections();
						?>
					</form>
					<?php

					// okay, open your eyes again
					$this->_replacements['create_form'] = ob_get_contents();
				ob_end_clean();
			}

			$this->_printContents();

		}//end if logged in

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
			$lock_errors = $hh->freestlyeHipo('hipo_job_acquire_lock', $vars);


			  /////////////////////////
			 //  EDIT DETAILS FORM  //
			/////////////////////////

			ob_start();
				if (!empty($lock_errors)) {
					$this->_replacements['edit_details_lock_error'] = $this->attr('edit_details_lock_error');
					$current_user->printBody();
				} else {
					$this->_replacements['edit_details_invite'] = $this->attr('edit_details_invite');
					$current_user->printLimbo();
				}
				$this->_replacements['edit_form'] = ob_get_contents();
			ob_end_clean();


			  ////////////////////
			 //  LOG OUT FORM  //
			////////////////////

			ob_start();
				?>
				<form name="account_manager_logout" method="POST" action="<?php echo current_url(true, true); ?>?SQ_ACTION=logout">
					<?php echo submit_button('ACCOUNT_MANAGER_ACTION', 'Logout'); ?></td>
				</form>
				<?php
				$this->_replacements['logout_form'] = ob_get_contents();
			ob_end_clean();
		}

		$this->_printContents();

	}//end printLimbo()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object Backend_Outputter	$o
	* @param array(string)				&$link	information used to create the initial link
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
					$o->openRaw();
					$this->_replacements['edit_details_success'] = $this->attr('edit_details_success');
					$o->closeRaw();
				}
			}

			return true;
		}

		return parent::processBackend($o, $link);

	}//end processBackend()


	/**
	* Print the contents of the Account Manager page with replaced keywords
	*
	* @return boolean
	* @access public
	*/
	function _printContents()
	{
		// get the bodycopy attached to us
		$bodycopy_data = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, $this->_current_state, 'major', '1');
		if (empty($bodycopy_data)) return;
		$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_data['minorid'], $bodycopy_data['minor_type_code']);
		if (is_null($bodycopy)) return;

		ob_start();
			$bodycopy->printBody();
			$html = ob_get_contents();
		ob_end_clean();
		$html = replace_keywords($html, $this->_replacements);

		// get rid of any keywords that have not been replaced
		$blanks = retrieve_keywords_replacements($html);
		$replacements = Array();
		foreach ($blanks as $keyword) $replacements[$keyword] = '';
		echo replace_keywords($html, $replacements);

	}//end _printContents()


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
	* This function captures all errors and warnings that occur to hide them from the user
	*
	* @param int	$err_no		The type of error (E_*)
	* @param string	$err_msg	The error message
	* @param string	$err_file	The file the error occured in
	* @param string	$err_line	The line the error occured on
	*
	* @returns void
	* @access private
	* @see HIPO_Job::process()
	*/
	function _errorHandler($err_no, $err_msg, $err_file, $err_line)
	{
		$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

		// if the function didn't have an '@' prepended OR if we are about to terminate
		// catch the error
		if (error_reporting() || $terminate) {

			// Strip out the file path begining
			$err_file = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_file);
			$err_msg  = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_msg);

			$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));

			// send a report to the system error log
			if (ini_get('log_errors')) {
				$err_name = '';
				switch($err_no) {
					case E_USER_ERROR	: $err_name = SQ_SYSTEM_SHORT_NAME.' Error';	break;
					case E_USER_WARNING	: $err_name = SQ_SYSTEM_SHORT_NAME.' Warning';	break;
					case E_USER_NOTICE	: $err_name = SQ_SYSTEM_SHORT_NAME.' Notice';	break;
					case E_ERROR		: $err_name = 'PHP Error';						break;
					case E_WARNING		: $err_name = 'PHP Warning';					break;
					case E_NOTICE		: $err_name = 'PHP Notice';						break;
					default				: $err_name = 'Unknown Error Type - '.$err_no;
				}

				$log_msg = date('D d/m/Y H:i:s').' ['.strtolower($err_name).'] ('.$err_file.':'.$err_line.') - '.$text_msg;
				$log_msg = str_replace('[SYSTEM_ROOT]', '', $log_msg);
				error_log($log_msg);
			}

			$this->_errors[] = $text_msg;

		}// end error_reporting

	}// end _errorHandler()


}//end class

?>