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
* $Id: page_asset_builder.inc,v 1.5 2004/01/23 04:38:11 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Asset_Builder
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Builder extends Page
{

	/**
	* The current state the asset builder is in
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
	* An array of keywords that this asset makes available for use along with a default replacements
	* @var Array()
	*/
	var $_available_keywords = Array(
								'login_invite',
								'login_form',
								'login_error',
								'create_invite',
								'create_form',
								'create_location',
								'create_error',
								);

	/**
	* An array of bodycopies that need to be created
	* @var Array()
	*/
	var $_bodycopies = Array('Not Logged In' => 'not_logged_in',
							 'Created'       => 'created',
							 'Logged In'     => 'logged_in',
							);


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Asset_Builder($assetid=0)
	{
		$this->_ser_attrs = true;
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

		foreach ($this->_bodycopies as $name => $value) {
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
		// if this is a bodycopy then we need to make it a dependant link
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
		// the only reason why we would object to this link being deleted is if it is a bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', null, '1', '1');

		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the bodycopy and the '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' page';

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link types, the asset type and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => count($this->_bodycopies), 'exclusive' => true);
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
		$this->_current_state = 'logged_in';
		$current_user = &$GLOBALS['SQ_SYSTEM']->user;

		$this->_processGlobalActions();
		// if a new asset is created, it will be stored in the tmp vars for use by other asset types
		// that extend this one, but we wont need it any more - so chuck it out
		if (isset($this->_tmp['created_asset'])) unset($this->_tmp['created_asset']);

		if (is_null($current_user)) {
			// this person is not logged in, so we should let them log in
			$this->_current_state = 'not_logged_in';
			$this->_loadLoginReplacements();
		}

		$this->_loadCreateReplacements();
		$this->_printBody();

	}//end printBody()


	/**
	* Prints the Asset Builder form and contents
	*
	* @return void
	* @access public
	*/
	function _printBody()
	{
		?>
		<script type="text/javascript" language="javascript" src="<?php echo sq_web_path('lib');?>/html_form/html_form.js"></script>
		<form name="asset_builder" method="POST" action="<?php echo $_SERVER['PHP_SELF']; ?>" enctype="multipart/form-data">
			<?php
			echo hidden_field('ASSET_BUILDER_ACTION', 'create');
			$this->_printContents();
			?>
		</form>
		<?php

	}//end _printBody()


	/**
	* Process any global Asset Builder actions
	*
	* These actions include logging into the system and creating an asset
	*
	* @return void
	* @access private
	*/
	function _processGlobalActions()
	{
		// process some global asset builder actions
		if (isset($_POST['ASSET_BUILDER_ACTION'])) {

			switch (strtolower($_POST['ASSET_BUILDER_ACTION'])) {

				case 'login' :
					// they have tried logging in - but failed
					$this->_replacements['login_error'] = $this->attr('login_error');
				break;

				case 'create' :

					if (!isset($_POST['asset_create_location'])) {
						trigger_error('No asset create location specified', E_USER_WARNING);
						break;
					}

					// we'll just use a limbo outputter to do the outputting for us
					require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
					$o = new Limbo_Outputter();

					if (is_array($_POST['asset_create_location'])) {
						$create_parentid = array_shift(array_values($_POST['asset_create_location']));
					} else {
						$create_parentid = (int)$_POST['asset_create_location'];
					}

					$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($create_parentid);
					if (is_null($parent)) {
						$this->_replacements['create_error'] = $this->attr('create_error');
						break;
					}

					$create_type = $this->attr('create_type');
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
					$new_asset = new $create_type();

					// start the transaction to create the asset
					$link = Array('asset'		=> &$parent,
								  'link_type'	=> $this->attr('create_link_type'),
								  'value'		=> '',
								  'sort_order'	=> '');

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					// set the install flag because we are creating an asset without being
					// logged in - so any write access checks will fail
					$GLOBALS['SQ_INSTALL'] = true;

						set_error_handler(Array(&$this, '_errorHandler'));
						if (!$new_asset->processBackend($o, $link)) {

							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							restore_error_handler();

						} else {

							// if we have multiple locations to create this asset under, create
							// the new links to all the other parents
							if (is_array($_POST['asset_create_location'])) {
								foreach (array_values($_POST['asset_create_location']) as $link_parentid) {
									$link_parentid = (int)$link_parentid;

									// skip the orginal parent we linked the new asset to
									if ($link_parentid == $parent->id) continue;

									$link_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_parentid, '', true);
									if (is_null($link_parent)) continue;
									if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($link_parent, $new_asset, $this->attr('create_link_type'))) {
										$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
									}
								}
							}

							if (empty($this->_errors)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
								$this->_current_state = 'created';
								restore_error_handler();

								$create_status = $this->attr('create_status');

								if ($new_asset->status != $create_status) {
									$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
									$vars = Array('assetid' => $new_asset->id, 'new_status' => $create_status);
									$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
									$this->_errors = array_merge($this->_errors, $status_errors);
								}
								$this->_tmp['created_asset'] = $new_asset;
							}

						}//end if processBackend

						// create a friendly error message
						$errors = '';
						foreach ($this->_errors as $text) $errors .= '<li>'.$text.'</li>';
						if (!empty($errors)) $this->_replacements['create_error'] = $this->attr('create_error').'<ul>'.$errors.'</ul>';

					$GLOBALS['SQ_INSTALL'] = false;

				break;

			}//end switch

		}//end if ASSET_BUILDER_ACTION

	}//end _processGlobalActions()


	/**
	* Loads keyword replacements for the logging in interface, into the Asset Builder
	*
	* @return void
	* @access private
	*/
	function _loadLoginReplacements()
	{
		$this->_replacements['login_invite'] = $this->attr('login_invite');

		ob_start();
			?>
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
					<td valign="top"><?php echo submit_button('ASSET_BUILDER_ACTION', 'Login', 'this.form.action = "'.$_SERVER['PHP_SELF'].'?SQ_ACTION=login"; set_hidden_field("ASSET_BUILDER_ACTION", "login", document.asset_builder);'); ?></td>
				</tr>
			</table>

			<?php
			$this->_replacements['login_form'] = ob_get_contents();
		ob_end_clean();

	}//end _loadLoginReplacements()


	/**
	* Loads keyword replacements for the create asset interface, into the Asset Builder
	*
	* @return void
	* @access private
	*/
	function _loadCreateReplacements()
	{
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

			// used to indicate to the edit interface that we are creating an asset
			?><input type="hidden" name="asset_action" value="create"><?php

			// paint the buffered sections
			$o->paintSections();

			// okay, open your eyes again
			$this->_replacements['create_form'] = ob_get_contents();
		ob_end_clean();

		ob_start();
			$create_locations = $this->attr('create_location');
			if (empty($create_locations)) {
				bam('EMBEDDED ASSET MAP WILL GO HERE WITH AN ASSET FINDER');
			} else if (count($create_locations) == 1) {
				$create_location = array_pop($create_locations);
				?><input type="hidden" name="asset_create_location" value="<?php echo $create_location; ?>"><?php
			} else {
				$options = Array();
				$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($create_locations));
				foreach ($asset_info as $info) $options[$info['assetid']] = $info['name'];
				if ($this->attr('create_location_type') == 'select') {
					combo_box('asset_create_location', $options, (bool)$this->attr('create_location_multiple'), 0);
				} else {
					$multiple = (bool)$this->attr('create_location_multiple');
					echo '<ul style="list-style-type:none;">';
					foreach ($options as $assetid => $name) {
						echo '<li>';
						if ($multiple) check_box('asset_create_location[]', $assetid);
						else radio_button('asset_create_location', $assetid);
						echo $name.'</li>';
					}
					echo '</ul>';
				}
			}

			$this->_replacements['create_location'] = ob_get_contents();
		ob_end_clean();

	}//end _loadCreateReplacements()


	/**
	* Print the contents of the Asset Builder page with replaced keywords
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

		$bodycopy->setKeywordReplacements($this->_replacements);
		$bodycopy->printBody();

	}//end _printContents()


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

	}//end _errorHandler()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
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
			if ($keyword == 'create_location') {
				$default = '<select><option>Example Location 1</option><option>Example Location 2</option></select>';
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
