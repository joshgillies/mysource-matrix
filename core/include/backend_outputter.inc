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
* $Id: backend_outputter.inc,v 1.176 2013/10/22 00:38:23 lwright Exp $
*
*/


require_once SQ_LIB_PATH.'/html_form/html_form.inc';

// just normal messages, eg update confirmations
define('SQ_BO_MSG_NOTICE',  1);
// warnings, you entered something a bit a-miss and the system fixed it up for you
define('SQ_BO_MSG_WARNING', 2);
// something is just plain wrong, fix immediately
define('SQ_BO_MSG_ERROR',   4);


/**
* 
*
* Purpose
*
*    This class will be controlling all output for the backend of thesystem,
*    there will only ever be one instance of this object
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.176 $
* @package MySource_Matrix
*/
class Backend_Outputter
{

	// any messages to be displayed to the user
	var $_messages = Array();

	// an link to redirect to when we print, any messages will be carried over to the next page
	var $_redirect = '';
	var $_redirect_target = '';

	// an array of fns to be called when the page loads
	var $_on_load_calls = Array();

	// an array assets that are going to be refreshed in the flash menu
	var $_refreshing_assets = Array();

	// the form action
	var $_action  = '';

	// form GET vars that the backend outputter will endevour to ensure are on each screen
	var $_persistant_get_vars = Array();

	// the anchor the form will be submitted to
	var $_anchor  = '';

	// the page heading
	var $_page_title = '';
	var $_heading  = '';
	var $_heading_icon = '';
	var $_sub_heading = '';

	// stylesheet files to include
	var $_css_includes = Array();

	// javascript files to include
	var $_js_includes = Array();

	// an array of fns to be called when the form is submitted
	var $_on_submit_calls = Array();

	// hidden fields for the form Array([field name] => [value])
	var $_hidden_fields = Array();

	// edit screens
	var $_screens = Array();
	var $_static_screens = Array();

	// the contents of the page split into sections
	var $_contents = Array();

	// used to make sure that each img gets a unique name
	var $_img_button_count = 0;

	// all the preload images names on the page and their preloaded images
	// Array(name => img);
	var $_preload_imgs = Array();

	var $_files_path;

	var $_raw_opened = FALSE;

	var $_section_stack = Array();
	var $_current_field = NULL;

	var $_section_count = 0;
	var $_field_count   = 0;

	var $_charset = '';

	/**
	* If TRUE all output will be printed when paint is called
	* @var boolean
	*/
	var $_buffering = TRUE;

	/**
	* If TRUE there is a section opened (used when we are not buffering)
	* @var boolean
	*/
	var $_is_open_section = FALSE;

	/**
	* If TRUE there is a field opened (used when we are not buffering)
	* @var boolean
	*/
	var $_is_open_field = FALSE;

	/**
	* If type of the current field being painted (used when we are not buffering)
	* @var string
	*/
	var $_current_field_type = '';

	/**
	* The note of the current field being printed (used when we are not buffering)
	* @var string
	*/
	var $_current_field_note = '';

	/**
	* we need a separate section counter for non-buffering
	* @var integer
	*/
	var $_nb_section_count = 0;


	/**
	* Constructor
	*
	* @access  public
	*/
	function Backend_Outputter()
	{

		// if there are some messages from the previous page we had better add them in
		if (!empty($_SESSION['backend_outputter_msgs'])) {
			$this->_messages = $_SESSION['backend_outputter_msgs'];
			$_SESSION['backend_outputter_msgs'] = Array();
		}

		$this->_files_path = sq_web_path('lib').'/web';
		$this->_action = $_SERVER['PHP_SELF'];

		// register that we want to listen to link update events
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();

		$em->addEventListener($this, Array(
										'CreateLink',
										'DeleteLink',
										'AssetUpdate',
										'AssetTypeUpdate',
										'BackendMessage',
										'AssetStatusUpdate',
									 )
							);

	}//end constructor


	/**
	* Sets whether the backend output will buffer its output
	*
	* @param boolean	$buffer	if TRUE buffering will be turned on (on by default)
	*
	* @return boolean
	* @access public
	*/
	function setBuffering($buffer=TRUE)
	{
		$this->_buffering = (bool) $buffer;
		return TRUE;

	}//end setBuffering()


	/**
	* Set the redirect for the page
	*
	* @param string	$link	URL to redirect to
	* @param string	$target	Frame to redirect in
	*
	* @return void
	* @access public
	*/
	function setRedirect($link, $target='self')
	{
		// ensure that any persistant GET vars are appended to the redirect URL
		$link_extras = '';
		foreach ($this->_persistant_get_vars as $name => $value) {
			$link_extras .= '&'.rawurlencode($name).'='.rawurlencode($value);
		}
		$link_extras = trim($link_extras, ' &');
		if (!empty($link_extras)) {
			$link .= ((strstr($link, '?')) ? '&' : '?').$link_extras;
		}

		$this->_redirect = trim($link);
		$this->_redirect_target = trim($target);

	}//end setRedirect()


	/**
	* Add an onLoad call, something JS that needs to be executed right after the page finishes loading
	*
	* @param string	$call	JS code to execute
	*
	* @return void
	* @access public
	*/
	function addOnLoad($call)
	{
		$this->_on_load_calls[] = $call;

	}//end addOnLoad()


	/**
	* Allows you to add get vars to the action section of the form
	*
	* @param string		$name		the name of the var
	* @param string		$value		the value of the var
	* @param boolean	$persistant	try and keep this var available at all times (even on redirects)?
	*
	* @return void
	* @access public
	*/
	function addFormActionGetVar($name, $value, $persistant=FALSE)
	{
		if (FALSE === strpos($this->_action, '?')) {
			$url = $this->_action;
			$query = '';
		} else {
			list($url, $query) = explode('?', $this->_action);
		}
		$this->_action = replace_query_string_vars(Array($name => rawurlencode($value)), $url, $query);
		if ($persistant) {
			$this->_persistant_get_vars[$name] = $value;
		}

	}//end addFormActionGetVar()


	/**
	* Returns the current action, which realistically is going to be the way to get to the current location
	*
	* @return void
	* @access public
	*/
	function getCurrentLocation()
	{
		return $this->_action;

	}//end getCurrentLocation()


	/**
	* Set the anchor that the page will submit to
	*
	* @param string	$anchor	Anchor name
	*
	* @return void
	* @access public
	*/
	function setFormAnchor($anchor)
	{
		$anchor = trim($anchor);
		$this->_anchor = ($anchor) ? '//'.rawurlencode($anchor) : '';

	}//end setFormAnchor()


	/**
	 * Set the page title
	 * @param $title the page title
	 * @return void
	 * @access private
	 */
	function setPageTitle($title)
	{
	  $this->_page_title = trim($title);

	}//end setPageTitle()


	/**
	* Set the heading for page
	*
	* @param string	$heading	the heading to display
	* @param string	$icon		the icon img tag (if any)
	*
	* @return void
	* @access public
	*/
	function setHeading($heading, $icon='')
	{
		$this->_heading = trim($heading);
		$this->_heading_icon = $icon;

	}//end setHeading()


	/**
	* Set the sub heading for page
	*
	* @param string	$heading	the heading to display
	* @param string	$icon		the icon img tag (if any)
	*
	* @return void
	* @access public
	*/
	function setSubHeading($heading, $icon='')
	{
		$this->_sub_heading = trim($heading);

	}//end setSubHeading()


	/**
	* Add a message to be displayed to the user
	*
	* @param string	$type	Type of message
	* @param string	$msg	Message contents
	*
	* @return void
	* @access public
	*/
	function addMessage($type, $msg)
	{
		$msg = trim($msg);
		// if the type is valid and there is a message
		if (strlen($msg)) {
			$this->_messages[] = Array('type' => $type, 'msg' => $msg);
		}

	}//end addMessage()


	/**
	* Add a stylesheet file to the page
	*
	* @param string	$file	URL of the CSS file
	*
	* @return void
	* @access public
	*/
	function addCssInclude($file)
	{
		if (in_array($file, $this->_css_includes)) return;
		$this->_css_includes[] = $file;

	}//end addCssInclude()


	/**
	* Add a javascript file to the page
	*
	* @param string	$file	URL of the JS file
	*
	* @return void
	* @access public
	*/
	function addJsInclude($file)
	{
		if (in_array($file, $this->_js_includes)) return;
		$this->_js_includes[] = $file;

	}//end addJsInclude()


	/**
	* Add an onSubmit call, some JS that needs to be executed before the form submits
	*
	* @param string	$call	The JS code
	*
	* @return void
	* @access public
	*/
	function addOnSubmit($call)
	{
		$this->_on_submit_calls[] = $call;

	}//end addOnSubmit()


	/**
	* Add a hidden field to the output
	*
	* @param string	$name	the name of the field
	* @param string	$value	the value to store in the field
	*
	* @return void
	* @access public
	*/
	function addHiddenField($name, $value='')
	{
		$this->_hidden_fields[$name] = $value;

	}//end addHiddenField()


	/**
	* Adds a screen options to the drop down
	*
	* @param string	$url	the url to go to when this screen is selected
	* @param string	$name	the name to display in the drop down
	*
	* @return void
	* @access public
	*/
	function addScreen($url, $name)
	{
		if (empty($url)) return;
		if (in_array($name, $this->_screens)) {
			trigger_localised_error('SYS0202', E_USER_ERROR, $name);
		}
		$this->_screens[$url] = $name;

	}//end addScreen()


	/**
	* Adds a static screen for editing
	*
	* @param string	$url	The URL of the static screen
	* @param string	$name	The friendly name of the static screen
	*
	* @return void
	* @access public
	*/
	function addStaticScreen($url, $name)
	{
		if (empty($url)) return;
		if (in_array($name, $this->_static_screens)) {
			trigger_localised_error('SYS0207', E_USER_ERROR, $name);
		}
		$this->_static_screens[$url] = $name;

	}//end addStaticScreen()


	/**
	* Adds a preload image
	*
	* @param string	$file	The URL of the image
	*
	* @return void
	* @access public
	*/
	function addPreloadImg($file)
	{
		$this->_preload_imgs[] = $file;

	}//end addPreloadImg()


	/**
	* Set the character set for the page
	*
	* @param string	$charset	the character set to use
	*
	* @return void
	* @access public
	*/
	function setCharset($charset)
	{
		$this->_charset = $charset;

	}//end setCharset()


	/**
	* Opens a section of the page for raw output
	*
	* @return void
	* @access public
	*/
	function openRaw()
	{
		$init_arr = Array(
						'area_type'	=> 'section',
						'type'		=> 'raw',
						'contents'	=> '',
					);

		$ret_val = $this->_openSection($init_arr);
		ob_start();
		$this->_raw_opened = TRUE;
		return $ret_val;

	}//end openRaw()


	/**
	* Closes a raw output section
	*
	* @return void
	* @access public
	*/
	function closeRaw()
	{
		if (!$this->_raw_opened) return;
		$stack_i = count($this->_section_stack) - 1;
		$this->_section_stack[$stack_i]['contents'] = ob_get_contents();
		ob_end_clean();
		$this->_raw_opened = FALSE;
		$this->_closeSection();

	}//end closeRaw()


	/**
	* Opens a section
	*
	* @param string	$heading	The heading name
	* @param string	$icon		The heading icon
	*
	* @return void
	* @access public
	*/
	function openSection($heading='', $icon='')
	{
		$init_arr = Array(
						'area_type'	=> 'section',
						'type'		=> 'normal',
						'heading'	=> $heading,
						'icon'		=> $icon,
						'areas'		=> Array(),
					);
		return $this->_openSection($init_arr);

	}//end openSection()


	/**
	* Closes a section
	*
	* @return void
	* @access public
	*/
	function closeSection()
	{
		if (!is_null($this->_current_field)) {
			$this->closeField();
		}

		if ($this->_buffering) {
			$this->_closeSection();
		} else {
			$this->_nbCloseSection();
			$this->_nb_section_count--;
			if ($this->_nb_section_count == 0) {
				$this->_is_open_section = FALSE;
			}
		}

	}//end closeSection()


	/**
	* Opens a section, with the passed initialise array
	*
	* @param array	$init_arr	some initialising data
	*
	* @return void
	* @access private
	*/
	function _openSection($init_arr)
	{
		if ($this->_is_open_field) $this->closeField();

		if ($this->_buffering) {
			if (!is_null($this->_current_field)) {
				$this->closeField();
			}
			if ($this->_raw_opened) $this->closeRaw();

			$this->_section_count++;
			$init_arr['section_count'] = $this->_section_count;

			// Nothing in the stack ? we are in the top level
			if (empty($this->_section_stack)) {
				$j = count($this->_contents);
				$this->_contents[$j] = $init_arr;
				$this->_section_stack[] =& $this->_contents[$j];
			} else {
				$i = count($this->_section_stack) - 1;
				$j = count($this->_section_stack[$i]['areas']);
				$this->_section_stack[$i]['areas'][$j] = $init_arr;
				$this->_section_stack[] =& $this->_section_stack[$i]['areas'][$j];
			}

			return 'section_'.$this->_section_count;

		} else {
			$this->_nbOpenSection($init_arr);
		}

	}//end _openSection()


	/**
	* Opens a section without buffering
	*
	* @param array	$init_arr	some initialising data
	*
	* @return void
	* @access private
	*/
	function _nbOpenSection($init_arr)
	{
		if ($this->_nb_section_count > 0) echo '<tr><td>';

		$this->_nb_section_count++;

		?>
		<a name="section_<?php echo $this->_nb_section_count; ?>"></a>
		<table class="sq-backend-section-table">
		<?php
		if (isset($init_arr['heading']) && $init_arr['heading']) {
			if ($this->_is_open_section) {
				// printing a nested section - needs to look a little different
				?>
				<tr>
					<td class="sq-backend-section-subheading"><?php echo $init_arr['heading'];?></td>
				</tr>
				<?php
			} else {
				$this->_is_open_section = TRUE;
				// printing a top level section
				?>
				<tr class="sq-backend-section-heading-container">
					<td>
						<table>
							<tr>
								<td class="sq-backend-section-heading"><?php echo $init_arr['heading'];?></td>
							</tr>
						</table>
					</td>
				</tr>
				<?php
			}//endif
		}//endif

		?>
			<tr>
				<td class="sq-backend-section-inner-table-wrapper">
					<table class="sq-backend-section-table-inner">
		<?php


	}//end _nbOpenSection()


	/**
	* Closes a section
	*
	* @return void
	* @access private
	*/
	function _closeSection()
	{
		if ($this->_buffering) {
			array_pop($this->_section_stack);
		} else {
			$this->_nbCloseSection();
		}

	}//end _closeSection()


	/**
	* Closes a section without buffering
	*
	* @return void
	* @access private
	*/
	function _nbCloseSection()
	{
		if ($this->_nb_section_count > 0) echo '</td></tr>';

		?>
					</table>
				</td>
			</tr>
		</table>
		<?php

	}//end _nbCloseSection()


	/**
	* Opens a field inside a section
	*
	* @param string	$name		label of the field
	* @param string	$format		format 'new_line' | 'blank' | 'commit' | ''
	* @param string	$note		a note to place with the field
	* @param string	$hidden		whether to hide the field inially or not
	* @param string	$id_name	the id name to give the element that we hide (so it can be found easily)
	*
	* @return void
	* @access public
	*/
	function openField($name, $format='', $note='', $hidden=FALSE, $id_name='')
	{
		if ($this->_buffering) {
			if (!is_null($this->_current_field)) {
				$this->closeField();
			}
			if (empty($this->_section_stack)) $this->openSection();
			if (!trim($name)) $name = '&nbsp;';

			$this->_field_count++;

			$stack_i = count($this->_section_stack) - 1;
			$i = count($this->_section_stack[$stack_i]['areas']);
			$this->_section_stack[$stack_i]['areas'][$i] = Array(
															'area_type'		=> 'field',
															'type'			=> 'normal',
															'field_count'	=> $this->_field_count,
															'name'			=> $name,
															'format'		=> $format,
															'note'			=> $note,
															'contents'		=> '',
															'hidden'		=> $hidden,
															'id_name'		=> $id_name,
														   );

			$this->_current_field =& $this->_section_stack[$stack_i]['areas'][$i];
			ob_start();

			return 'field_'.$this->_field_count;

		} else {
			// we are not buffering
			$this->_nbOpenField($name, $format, $note, $hidden);
		}

	}//end openField()


	/**
	* Print a field without buffering (nb == non-buffering)
	*
	* @param string		$name	label of the field
	* @param string		$format	format 'new_line' | 'blank' | 'commit' | ''
	* @param string		$note	a note to place with the field
	* @param boolean	$hidden	Whether the field should be hidden
	*
	* @return boolean
	* @access private
	*/
	function _nbOpenField($name, $format='', $note='', $hidden=FALSE)
	{
		if ($this->_is_open_field) $this->closeField();
		$this->_is_open_field = TRUE;

		// store the format and note of the current field for future reference
		$this->_current_field_type = $format;
		$this->_current_field_note = $note;

		switch ($format) {
			case 'new_line' :
				?>
				<tr <?php if ($hidden) echo 'style="display: none"'; ?>>
					<td class="sq-backend-field sq-backend-field-newline" colspan="2">
						<a name="field_<?php echo $this->_field_count; ?>"></a>
						<span class="sq-backend-field-label"><?php echo $name;?></span>
					</td>
				</tr>
				<tr <?php if ($hidden) echo 'style="display: none"'; ?>>
					<td class="sq-backend-data sq-backend-data-newline" colspan="2">
				<?php
			break;

			case 'commit' :
				?>
				<tr <?php if ($hidden) echo 'style="display: none"'; ?>>
					<td class="sq-backend-data" style="text-align: right;" colspan="2">
				<?php
			break;

			case 'blank' :
				// a blank field to be used just for formatting
			break;

			default :
				?>
				<tr <?php if ($hidden) echo 'style="display: none"'; ?>>
					<td class="sq-backend-field">
						<a name="field_<?php echo $this->_field_count; ?>"></a>
						<span class="sq-backend-field-label"><?php echo $name;?></span>
					</td>
					<td class="sq-backend-data">
				<?php
		}//end switch

	}//end _nbOpenField()


	/**
	* Closes a field
	*
	* @return void
	* @access public
	*/
	function closeField()
	{

		if ($this->_buffering) {
			if (is_null($this->_current_field)) return;
			$this->_current_field['contents'] = ob_get_contents();
			ob_end_clean();
			// unset the reference before setting to null, so we don't blow away the field's array
			unset($this->_current_field);
			$this->_current_field = NULL;
		} else {
			$this->_nbCloseField();
		}

	}//end closeField()


	/**
	* Closes a field without buffering
	*
	* @return boolean
	* @access private
	*/
	function _nbCloseField()
	{
		$this->_is_open_field = FALSE;

		// We need to end the field that we started
		// Because they all start differently, we have to end what we started
		switch ($this->_current_field_type) {

			case 'blank' :
				// a blank field to be used just for formatting
			break;

			default :
				if ($this->_current_field_note) {
					$this->note($this->_current_field_note);
				}
				?>
					</td>
				</tr>
				<?php
		}//end switch

	}//end _nbCloseField()


	/**
	* Returns the path to the web dir in lib
	*
	* @param string	$file	if supplied is concatenated onto the path before returning
	*
	* @return string
	* @access public
	*/
	function filesPath($file='')
	{
		if ($file && substr($file, 0, 1) != '/') {
			$file = '/'.$file;
		}
		return $this->_files_path.$file;

	}//end filesPath()


	/**
	* Paint the cached backend interface
	*
	* @return void
	* @access public
	*/
	function paint()
	{
		if ($this->_raw_opened) $this->closeRaw();
		// close all sections
		while (!empty($this->_section_stack)) {
			$this->closeSection();
		}

		if ($this->_redirect) {
			// save the messages so they come through on the next page
			$_SESSION['backend_outputter_msgs'] = $this->_messages;
			$this->addOnLoad($this->_redirect_target.'.location = "'.addslashes($this->_redirect).'";');
		}

		$this->_paintHeader();

		if (!$this->_redirect) {
			for ($i = 0; $i < count($this->_contents); $i++) {
				$this->_paintSection($this->_contents[$i]);
			}
		}

		$this->_paintFooter();

	}//end paint()


	/**
	* Returns the outputters painted contents as a string
	*
	* @return string
	* @access public
	*/
	function getAsString()
	{
		ob_start();
			$this->paint();
			$str = ob_get_contents();
		ob_end_clean();

		return $str;

	}//end getAsString()


	/**
	* Paint the header of the page
	*
	* @return void
	* @access private
	*/
	function _paintHeader()
	{
		if (!headers_sent()) {
			if ($this->_charset) {
				header('Content-type: text/html; charset='.$this->_charset);
			} else {
				header('Content-type: text/html; charset='.SQ_CONF_DEFAULT_CHARACTER_SET);
			}
		}

		$backendPage = array_get_index($_GET, 'SQ_BACKEND_PAGE');

		// If we are the sidenav page, push pages into standards mode where possible
		if ($backendPage === 'sidenav') {
			?><!DOCTYPE html>
		<?php } ?>
		<html>
		<head>
			<title><?php echo htmlspecialchars(str_replace('&nbsp;', ' ', $this->_page_title)); ?></title>
			<!--link rel="stylesheet" type="text/css" href="http://bbanda.staging.squiz.net/?a=73"/-->
			<?php
			$this->addCssInclude($this->filesPath('css/edit.css'));
			if (!empty($_REQUEST['print_view'])) {
				$this->addCssInclude($this->filesPath('css/print.css'));
				$this->addOnLoad('pagePrint()');
			}

			foreach ($this->_css_includes as $file) {
				$this->_paintCssInclude($file);
			}

			// Add IE6-specific styles if main frame AND "lock commit button to frame" preference turned on
			if (($backendPage == 'main') && SQ_IN_BACKEND && $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_COMMIT_BUTTON_POS')) {

				// Bug #3096 - Printing incorrectly in IE
				// Associated with the CSS involved with hanging the Commit Button to the bottom of the page
				// If print_view is not enabled show the commit button code
				if (!isset($_REQUEST['print_view'])) {
				?>
				<!--[if gte IE 6]>
					<link href="<?php echo $this->filesPath('css/edit_ie6.css') ?>" type="text/css" rel="stylesheet" />
				<![endif]-->
				<?php
				}//end if
			}

			// first add the js translation files
			foreach ($GLOBALS['SQ_SYSTEM']->lm->getJavascriptIncludes() as $js_include) {
				$this->addJsInclude($js_include);
			}

			// now other general stuff
			$this->addJsInclude(sq_web_path('lib').'/html_form/html_form.js');
			$this->addJsInclude(sq_web_path('lib').'/js/general.js');
			$this->addJsInclude(sq_web_path('lib').'/js/backend_search.js');
			$this->addJsInclude(sq_web_path('lib').'/js/debug.js');
			$this->addJsInclude(sq_web_path('lib').'/js/edit.js');
			$this->addJsInclude(sq_web_path('lib').'/js/state.js');
			$this->addJsInclude(sq_web_path('lib').'/js/tooltip.js');
			$this->addOnLoad('fixIcons(\''.sq_web_path('lib').'/web/images/blank.gif\');');

			// and paint them all out
			foreach ($this->_js_includes as $file) {
				$this->_paintJsInclude($file);
			}
			?>
			<script type="text/javascript"><!--
			//<![CDATA[
				var SQ_DOCUMENT_LOADED = false;
				function page_on_load()
				{
					<?php
					// If you click on a window outside of the asset map frame,
					// clear out any menus from the (JS) asset map.
					if ($backendPage !== 'sidenav') {
					?>
						window.onclick = function(event) {
							event = event || window.event;
							var sidenavFrame = top.frames.sq_sidenav;
							if (sidenavFrame.JS_Asset_Map) {
								sidenavFrame.JS_Asset_Map.clearMenus();
							}
						}
					<?php
					}//end if

					if (isset($_SESSION['backend_outputter_msgs'])) {
						$this->_messages = array_merge($this->_messages, $_SESSION['backend_outputter_msgs']);
						$_SESSION['backend_outputter_msgs'] = Array();
					}
					if (count($this->_messages)) {
						require_once 'XML/Tree.php';
						require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
						$msgs = new XML_Tree();
						$msgs_root = $msgs->addRoot('messages');
						foreach ($this->_messages as $data) {
							$msg_body = '<p>'.htmlentities(nl2br($data['msg']), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'</p>';
							$msg_body .= '<span style="color: #333333">'.$GLOBALS['SQ_SYSTEM']->datetime().'<span>';
							$msgs_root->addChild('message', $msg_body, Array('type' => get_bit_names('SQ_BO_MSG_', $data['type'])));
						}
						?>
						if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].add_messages) {
							parent.frames["sq_sidenav"].add_messages("<?php echo str_replace(Array("\r\n", "\n", "\r"), "\\n", addslashes($msgs->get())); ?>");
						}
						<?php
					}//end if messages
					?>

					if (window.self.name == 'sq_main') {
						tooltip.print();
					}

					<?php
					foreach ($this->_on_load_calls as $call) {
						echo "\n".$call;
					}
					?>

					// let everyone know the document is loaded
					SQ_DOCUMENT_LOADED = true;

					// if IE
					if (document.all) {
						if (document.getElementById('sq-commit-button-div')) {
							commit_div = document.getElementById('sq-commit-button-div');
							if (commit_div.currentStyle && (commit_div.currentStyle.position == 'absolute')) {
								document.body.style.paddingBottom = (commit_div.clientHeight + 2) + "px";
							}
						}
					} else {
						if (document.getElementById('sq-commit-button-div')) {
							//document.getElementById('sq-commit-button-spacer').style.height = (document.getElementById('sq-commit-button-div').clientHeight + 2) + "px";
						}
					}

					// Remember state
					resetChanges();
					document.forms[0]['state'].value = saveState();

				}//end page_on_load()


				function page_on_unload()
				{
					if (parent.frames['sq_sidenav'] && window.self.name == 'sq_main' && parent.frames['sq_sidenav'].asset_finder_onunload) {
						parent.frames['sq_sidenav'].asset_finder_onunload();
					}

				}//end page_on_unload()


				var SQ_FORM_SUBMITTED = false;
				var SQ_FORM_ERROR_CONTAINED = false;
				function form_on_submit()
				{
					if (SQ_FORM_SUBMITTED) {
						alert(js_translate('form_already_submitted'));
						return;
					}

					<?php
					foreach ($this->_on_submit_calls as $call) {
						echo $call.";\n";
					}
					?>

					// if the form contains errors and we don't want to submit, handle at here.
					if (SQ_FORM_ERROR_CONTAINED) {
						alert(js_translate('form_contains_error'));
						return;
					}

					// basically if they get this far then we can submit
					SQ_FORM_SUBMITTED = true;

					// If we are submitting the header frame, we are probably
					// submitting the search... so add "search in progress"
					// screen
					if (self.name == 'sq_header') {
						// Only pop-up the "search in progress" box if there's something to search on
						if (self.document.getElementById("asset_search").value != '') {
							top.frames["sq_main"].document.getElementById("sq-search-wait-popup").style.display = "block";
						}

						screenMenu = top.frames["sq_main"].document.getElementById("sq_screen_menu");
						screenMenuFiller = top.frames["sq_main"].document.getElementById("sq_screen_menu_filler");

						if (screenMenu) {
							if (window.navigator.userAgent.indexOf('MSIE') > 0) {
								screenMenuFiller.style.width = screenMenu.offsetWidth;
								screenMenuFiller.style.height = screenMenu.offsetHeight;
								screenMenuFiller.style.display = "block";
								screenMenu.style.display = "none";
							}
							screenMenu.style.MozOpacity = "0";
						}

						if (reload_timeout) {
							clearTimeout(reload_timeout);
						}
					}

					// disable all submit buttons
					disable_buttons();

					// Update the form state
					makeChanges();

					return true;

				}//end form_on_submit()

			<?php
			foreach ($this->_preload_imgs as $file) {
				?>
				preload_image('<?php echo $file;?>');
				<?php
			}//end foreach
			?>

			//]]> -->
			</script>
		</head>
		<body onload="page_on_load();" onunload="page_on_unload();" style="background: #F4F4F4" class="<?php echo $backendPage; ?>">
		<div id="sq-search-wait-popup"><div id="sq-search-wait-popup-titlebar"><div id="sq-search-wait-popup-close">[ <a href="#" onclick="document.getElementById('sq-search-wait-popup').style.display = 'none'; return false;">x</a> ]</div><span id="sq-search-wait-popup-title">Search in Progress</span></div>
			<div id="sq-search-wait-popup-details">Your search is being processed, please wait...</div>
		</div>
		<form action="<?php echo $this->_action.$this->_anchor;?>" id="main_form" name="main_form" method="post" enctype="multipart/form-data" onclick="changesMade();" onkeypress="changesMade();" onsubmit="return form_on_submit();" class="<?php echo $backendPage; ?>">
		<?php
			if (!empty($_REQUEST['print_view'])) {
				?>
				<div id="matrix-print-logo">
				</div>
				<?php
			}
		?>
		<div id="sq-content" class="<?php echo $backendPage; ?>">
		<?php
		hidden_field('process_form', '1');
		hidden_field('changes', '0');
		hidden_field('state', '');
		hidden_field('allowconfirm', !defined('SQ_CONF_CONFIRM_SAVE_CHANGES') ? '1' : SQ_CONF_CONFIRM_SAVE_CHANGES);
		// insert nonce token
		hidden_field('token', get_unique_token());

		foreach ($this->_hidden_fields as $name => $value) {
			hidden_field($name, $value);
		}

		if ($this->_heading || count($this->_screens) || count($this->_static_screens)) {
			?>
			<div class="sq-backend-main-header-wrapper">
				<div class="sq-backend-main-header-top">
					<table class="sq-backend-main-headings">
						<tr>
							<?php
							if ($this->_heading_icon) {
								?>
								<td class="sq-backend-heading-icon"><?php echo $this->_heading_icon; ?></td>
								<?php
							}
							?>
							<td class="sq-backend-main-heading">
								<?php
									echo $this->_heading;
									if ($this->_sub_heading) {
										echo '<br /><span class="sq-backend-sub-heading">'.$this->_sub_heading.'</span>';
									}
								?>
							</td>
						<?php
						if (empty($_REQUEST['print_view'])) {
							?>
							<td class="sq-backend-screen-menu">
								<div id="sq_screen_menu_filler" style="display: none"></div>
								<div id="sq_screen_menu">
									<?php
									$this->printContextSwitcher();
									$this->printScreenMenu();
									sq_print_icon(sq_web_path('lib').'/web/images/icons/printer.png', 16, 16, translate('print_this_page'), NULL, 'onclick="showPrintPopup()" class="clickable" style="margin-bottom: -2px; margin-right: 4px"');
									?>
								</div>
							</td>
							<?php
						}
						?>
						</tr>
					</table>
				</div>
				<?php

				if (isset($_SESSION['sq_nav_history']) && empty($_REQUEST['print_view'])) {
					?>
					<div class="sq-backend-main-header-bottom">
						<table class="sq-backend-nav-history-container">
							<tr>
								<td class="sq-backend-nav-history-title">
									<?php sq_print_icon($this->filesPath('images/icons/history.png'), 20, 20, 'Navigation History'); ?>
									<?php echo translate('back_to'); ?>:
								</td>
								<td class="sq-backend-nav-history">
									<?php
									$nav_count = count($_SESSION['sq_nav_history']);
									if ($nav_count < 5) {
										$start = 0;
									} else {
										$start = $nav_count - 5;
									}

									for ($i = $start; $i < $nav_count; $i++) {
										if (!isset($_SESSION['sq_nav_history'][$i])) continue;
										$nav_details = $_SESSION['sq_nav_history'][$i];

										$nav_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($nav_details['assetid'], '', TRUE);
										if (is_null($nav_asset)) {
											array_splice($_SESSION['sq_nav_history'], $i, 1);
											continue;
										}
										?>
										<a href="<?php echo $nav_asset->getBackendHref($nav_details['screen']); ?>&sq_nav_goback=<?php echo $i; ?>">
											<?php
											sq_print_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($nav_asset->type()), 16, 16, htmlspecialchars($nav_asset->short_name));
											echo htmlspecialchars($nav_asset->short_name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
											?>
											<span> [<?php echo ($i + 1); ?>]</span>
										</a>
										<?php
										if ($i < ($nav_count -1)) {
											?> <span class="divider">+</span> &nbsp;<?php
										}

										$GLOBALS['SQ_SYSTEM']->am->forgetAsset($nav_asset);
										unset($nav_asset);
									}
								?>
								</td>
							</tr>
						</table>
					</div>
				<?php
				}//end if
				?>
			</div><!--.sq-backend-main-header-wrapper-->
		<?php
		}//end if
		?>
	<div class="sq-backend-body-wrapper <?php echo $backendPage; ?>">
	<?php
	}//end _paintHeader()


	/**
	* Paint the passed section onto the page
	*
	* @param array	$section	an array with some infor about the section
	* @param int	$depth		an indicator to the depth we are currently at used for nested sections
	*
	* @return void
	* @access private
	*/
	function _paintSection($section, $depth=0)
	{
		if ($section['type'] == 'raw') {
			echo $section['contents'];
		} else {
			$wrapper_class = '';
			if (isset($section['areas'][0]['format'])) {
				$wrapper_class = $section['areas'][0]['area_type'].' '.$section['areas'][0]['format'];
			}else{
				if (isset($section['areas'][0]['area_type'])) {
					$wrapper_class = $section['areas'][0]['area_type'];
				}
			}
			?>
			<a name="section_<?php echo $section['section_count']; ?>"></a>
			<div class="sq-backend-section-table-wrapper <?php echo $wrapper_class;?>">
				<table class="sq-backend-section-table" id="backend_section_<?php echo $section['section_count']; ?>">
				<?php
				if ($section['heading'] != '') {
					if ($depth > 0) {
						// printing a nested section - needs to look a little different
						?>
						<tr>
							<td class="sq-backend-section-subheading"><?php echo $section['heading'];?></td>
						</tr>
						<?php
					} else {
						// printing a top level section
						?>
						<tr class="sq-backend-section-heading-container">
							<td>
								<table>
									<tr>
										<td class="sq-backend-section-heading"><?php echo $section['heading'];?></td>
									</tr>
								</table>
							</td>
						</tr>
						<?php
					}
				}//endif
				?>
				<tr>
					<td class="sq-backend-section-inner-table-wrapper">
						<table class="sq-backend-section-table-inner">
				<?php
				for ($j = 0; $j < count($section['areas']); $j++) {
					switch ($section['areas'][$j]['area_type']) {
						case 'section' :
							?>
								<tr>
									<td colspan="2">
										<?php
										$this->_paintSection($section['areas'][$j], $depth + 1);
										?>
									</td>
								</tr>
							<?php
						break;

						case 'field' :
							$field = $section['areas'][$j];
							switch ($field['format']) {
								case 'new_line' :
									?>
										<tr <?php
											if ($field['hidden']) echo 'style="display: none"';
											if ($field['id_name']) {
												echo 'id="'.$field['id_name'].'"';
											}
										?>>
											<td class="sq-backend-field sq-backend-field-newline" colspan="2">
												<a name="field_<?php echo $field['field_count']; ?>"></a>
												<span class="sq-backend-field-label"><?php echo $field['name'];?></span>
											</td>
										</tr>
										<tr <?php if ($field['hidden']) echo 'style="display: none"'; ?>>
											<td class="sq-backend-data sq-backend-data-newline" colspan="2">
												<?php
												echo $field['contents'];
												if ($field['note']) $this->note($field['note']);
												?>
											</td>
										</tr>
									<?php
								break;

								case 'commit' :
									?>
									<tr <?php
										if ($field['hidden']) echo 'style="display: none"';
										if ($field['id_name']) {
											echo 'id="'.$field['id_name'].'"';
										}
									?>>
										<td class="sq-backend-data sq-backend-commit" colspan="2">
											<?php
											echo $field['contents'];
											if ($field['note']) $this->note($field['note']);
											?>
										</td>
									</tr>
									<?php
								break;

								case 'blank' :
									// a blank field to be used just for formatting
								break;

								default :
								?>
									<tr <?php
										if ($field['hidden']) echo 'style="display: none"';
									?> <?php
										if ($field['id_name']) {
											echo 'id="'.$field['id_name'].'"';
										}
									?>>
										<td class="sq-backend-field">
											<a name="field_<?php echo $field['field_count']; ?>"></a>
											<span class="sq-backend-field-label"><?php echo $field['name'];?></span>
										</td>
										<td class="sq-backend-data">
											<?php
											echo $field['contents'];
											if ($field['note']) $this->note($field['note']);
											?>
										</td>
									</tr>
								<?php
							}//end switch

						break;

						default :
							trigger_localised_error('SYS0257', E_USER_ERROR, $section['areas'][$j]['area_type']);

					}//end switch area type

				}//end for

				if (isset($section['note'])) {
					$this->_nbSectionNote($section['note']);
				}

				?>
							</table>
						</td>
					</tr>
				</table>
			</div>
			<?php

		}//end else

	}//end _paintSection()


	/**
	* Paint the footer of the page
	*
	* @return void
	* @access private
	*/
	function _paintFooter()
	{
		?>
							</div><!--.sq-backend-body-wrapper-->
						</div><!--#sq-content-->
					</form>
				</body>
			</html>
		<?php

	}//end _paintFooter()


	/**
	* Outputs a stylesheet include line for the passed file
	*
	* @param string	$file	The URL of the file to include
	*
	* @return void
	* @access public
	*/
	function _paintCssInclude($file)
	{
		?>
			<link rel="stylesheet" type="text/css" href="<?php echo $file;?>" />
		<?php

	}//end _paintCssInclude()


	/**
	* Outputs a javascript include line for the passed file
	*
	* @param string	$file	The URL to the file
	*
	* @return void
	* @access public
	*/
	function _paintJsInclude($file)
	{
		?>
			<script type="text/javascript" src="<?php echo $file;?>"></script>
		<?php

	}//end _paintJsInclude()


	/**
	* Outputs a note in the proper css format
	*
	* @param string	$str	The note contents
	*
	* @return void
	* @access public
	*/
	function note($str)
	{
		echo '<div class="sq-backend-smallprint">'.$str.'</div>';

	}//end note()


	/**
	* Outputs a section note in the proper css format
	*
	* NOTE: a section can only have ONE note
	*
	* @param string	$str	The note contents
	*
	* @return void
	* @access public
	*/
	function sectionNote($str='')
	{
		if ($this->_buffering) {
			$stack_i = count($this->_section_stack) - 1;
			$this->_section_stack[$stack_i]['note'] = $str;

		} else {
			// we are not buffering
			$this->_nbSectionNote($str);
		}

	}//end sectionNote()


	/**
	* Outputs a section note in the proper css format
	*
	* NOTE: a section can only have ONE note
	*
	* @param string	$str	The note contents
	*
	* @return void
	* @access public
	*/
	function _nbSectionNote($str)
	{
		?>
				</td>
			</tr>
			<tr>
				<td colspan="2">
					<div class="sq-backend-section-note"><strong><?php echo translate('note'); ?>: </strong><?php echo $str ?></div>
		<?php

	}//end _nbSectionNote()


	/**
	* Creates a standardised commit button
	*
	* @param string		$value				The caption of the commit button, defaults to the
	*										Commit Button Text property or 'Commit' if not found
	* @param boolean	$release_lock		whether the lock should be released
	* @param boolean	$include_section	include the section
	*
	* @return void
	* @access public
	*/
	function commitButton($value='', $release_lock=TRUE, $include_section=TRUE)
	{
		// value is empty, use default caption
		if (empty($value)) {
			$value = !defined('SQ_CONF_COMMIT_BUTTON_TEXT') ? 'Commit' : SQ_CONF_COMMIT_BUTTON_TEXT;
		}
		$release_lock = (int) $release_lock;
		if ($include_section) {
			// Lock commit button to bottom of frame if the preference is turned on AND we're in backend
			if (SQ_IN_BACKEND && $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_COMMIT_BUTTON_POS')) {
				$this->openRaw();
				?></div>
				<div id="sq-commit-button-spacer"></div>
				<div id="sq-commit-button-div">
					<div id="sq-commit-button-div-inside">
						<?php
			} else {
				$this->openSection(' ');
				$this->openField('', 'commit');
			}
		}
		hidden_field('sq_lock_release', $release_lock);
		normal_button('sq_commit_button', $value, 'if (submit_form) { submit_form(this.form); } else { this.form.submit(); this.disabled = \'disabled\'; } ', 'accesskey="s" class="sq-form-field sq-btn-large sq-btn-green sq-commit-button"');
		if ($include_section) {
			if (SQ_IN_BACKEND && $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_COMMIT_BUTTON_POS')) {
				?>
					</div>
				</div>
				<div>
				<?php
				$this->closeRaw();
			} else {
				$this->closeField();
				$this->closeSection();
			}
		}

	}//end commitButton()


	/**
	* Prints the screen selection drop down list
	*
	* @return void
	* @access public
	*/
	function printScreenMenu()
	{
		if (empty($this->_screens) && empty($this->_static_screens)) {
			echo '&nbsp;';
		} else {
			if (!empty($this->_screens)) {
				$this->_screens = array_reverse($this->_screens);
				$this->_screens['#'] = '------';
				foreach (array_reverse($this->_static_screens) as $url => $code) {
					$this->_screens[$url] = $code;
				}
				$this->_screens = array_reverse($this->_screens);
			} else {
				$this->_screens = $this->_static_screens;
			}
			combo_box('screen_menu', $this->_screens, FALSE, $this->_action, 1, 'id="screen_menu" onchange="self.location = this.value"');
			echo '&nbsp;';
			normal_button('screen_menu_go', ' '.translate('go').' ', 'self.location = get_form_element_value("screen_menu");', 'class="sq-form-field sq-btn-shadow"');
			echo '&nbsp;';
		}

	}//end printScreenMenu()


	/**
	* Prints the interface for switching between contexts
	*
	* The context switcher will only be painted if there is
	* at least one alternate context set up (it does not need
	* to be valid for initial selection). If there is only one
	* context (ie. the default), nothing will be printed.
	*
	* @return void
	*/
	public function printContextSwitcher()
	{
		$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		$current_context = $GLOBALS['SQ_SYSTEM']->getContextId();
		if (count($contexts) > 1) {
			?>
			<span id="sq_context_switcher_current">
				Context: <strong><?php echo $contexts[$current_context]['name'] ?></strong> [ <a href="#" onclick="this.parentNode.nextSibling.style.display = 'inline'; this.parentNode.style.display = 'none'; return false;">Change</a> ]
			</span><span id="sq_context_switcher_change">Change Context: <?php
				$this->printSimpleContextSwitcher();
			?></span> <?php
		}

	}//end printContextSwitcher()


	/**
	* Print only the drop-down part of the context switcher
	*
	* It is expected that the "simple" version of the context
	* switcher will be used within Simple Edit interfaces,
	* in a similar way to the Screen Menu.
	*
	* @return void
	*/
	public function printSimpleContextSwitcher()
	{
		$current_location = $this->getCurrentLocation();
		list($current_loc_base, $current_loc_query) = explode('?', $current_location);
		$default_url = NULL;

		$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		$current_context = $GLOBALS['SQ_SYSTEM']->getContextId();
		if (count($contexts) > 1) {

			$context_list = Array();
			foreach ($contexts as $context_item_id => $context_item) {
				$url = replace_query_string_vars(Array('SQ_ACTION' => 'set_backend_context', 'SQ_CONTEXT_NAME' => rawurlencode($context_item['name'])), $current_loc_base, $current_loc_query);
				if ((int)$current_context === (int)$context_item_id) {
					$default_url = $url;
				}

				$context_list[$url] = $context_item['name'];
			}
			combo_box('sq_context_switcher', $context_list, FALSE, Array($default_url), 1, ' onchange="self.location = this.value;"');
		}


	}//end printSimpleContextSwitcher()


	/**
	* Event handler to add a message to the flash soeone broadcasts it
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$event_data		the vars that get submitted by the broadcaster
	*
	* @return void
	* @access public
	*/
	function onBackendMessage(&$broadcaster, $event_data=Array())
	{
		$this->addMessage($event_data['type'], $event_data['message']);

	}//end onBackendMessage()


	/**
	* Event handler to refresh the asset map if a new link has been created
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$event_data		the vars that get submitted by the broadcaster
	*
	* @return void
	* @access public
	*/
	function onCreateLink(&$broadcaster, $event_data=Array())
	{
		if (!is_null($broadcaster) && isset($broadcaster->id)) {
			if (!in_array($broadcaster->id, $this->_refreshing_assets)) {
				$this->addOnLoad('if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].reload_assets) parent.frames["sq_sidenav"].reload_assets("'.addslashes($broadcaster->id).'");');
				$this->_refreshing_assets[] = $broadcaster->id;
			}
		}//end if

	}//end onCreateLink()


	/**
	* Event handler to refresh the asset map if a link has been deleted
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$event_data		the vars that get submitted by the broadcaster
	*
	* @return void
	* @access public
	*/
	function onDeleteLink(&$broadcaster, $event_data=Array())
	{
		if (!($broadcaster instanceof Asset)) return;
		if (!in_array($broadcaster->id, $this->_refreshing_assets)) {
			$this->addOnLoad('if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].reload_assets) parent.frames["sq_sidenav"].reload_assets("'.addslashes($broadcaster->id).'");');
			$this->_refreshing_assets[] = $broadcaster->id;
		}

	}//end onDeleteLink()


	/**
	* Event handler to refresh the asset map if the short name of an asset has changed
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$event_data		the vars that get submitted by the broadcaster
	*
	* @return void
	* @access public
	*/
	function onAssetUpdate(&$broadcaster, $event_data=Array())
	{
		if (!($broadcaster instanceof Asset)) return;
		if (!in_array($broadcaster->id, $this->_refreshing_assets)) {
			$this->addOnLoad('if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].reload_assets) parent.frames["sq_sidenav"].reload_assets("'.addslashes($broadcaster->id).'");');
			$this->_refreshing_assets[] = $broadcaster->id;
		}

	}//end onAssetUpdate()


	/**
	* Event handler to refresh the asset map if an asset has been morphed to a new type
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$event_data		the vars that get submitted by the broadcaster
	*
	* @return void
	* @access public
	*/
	function onAssetTypeUpdate(&$broadcaster, $event_data=Array())
	{
		if (!($broadcaster instanceof Asset)) return;
		if (!in_array($broadcaster->id, $this->_refreshing_assets)) {
			if ($event_data['old_type'] != $event_data['new_type']) {
				$this->addOnLoad('if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].reload_assets) parent.frames["sq_sidenav"].reload_assets("'.addslashes($broadcaster->id).'");');
				$this->_refreshing_assets[] = $broadcaster->id;
			}
		}

	}//end onAssetTypeUpdate()


	/**
	* Event handler to refresh the asset map if an asset's status has changed
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$event_data		the vars that get submitted by the broadcaster
	*
	* @return void
	* @access public
	*/
	function onAssetStatusUpdate(&$broadcaster, $event_data=Array())
	{
		if (!($broadcaster instanceof Asset)) return;
		if (!in_array($broadcaster->id, $this->_refreshing_assets)) {
			if ($event_data['old_status'] != $event_data['new_status']) {
				$this->addOnLoad('if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].reload_assets) parent.frames["sq_sidenav"].reload_assets("'.addslashes($broadcaster->id).'");');
				$this->_refreshing_assets[] = $broadcaster->id;
			}
		}

	}//end onAssetStatusUpdate()


	/**
	* Get the section number we are up to
	*
	* @return int
	* @access public
	*/
	function getCurrentSectionNumber()
	{
		return $this->_section_count;

	}//end getCurrentSectionNumber()


}//end class

?>
