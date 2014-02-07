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
* $Id: limbo_outputter.inc,v 1.40 2012/09/06 05:41:58 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';

/**
* Limbo_Outputter
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.40 $
* @package MySource_Matrix
*/
class Limbo_Outputter extends Backend_Outputter
{


	/**
	* Constructor
	*
	* @access public
	*/
	function Limbo_Outputter()
	{
		$this->Backend_Outputter();

	}//end constructor


	/**
	* Allows you to add get vars to the action section of the form
	*
	* In limbo we dont want get vars though, so we will post all these
	*
	* @param string		$name		the name of the var
	* @param string		$value		the value of the var
	* @param boolean	$persistant	ignored
	*
	* @return void
	* @access public
	*/
	function addFormActionGetVar($name, $value, $persistant=FALSE)
	{
		if ($persistant) {
			return parent::addFormActionGetVar($name, $value, TRUE);
		}
		return $this->addHiddenField($name, $value);

	}//end addFormActionGetVar()


	/**
	* Paint the cached limbo interface
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
		$this->_paintBodyHeader();
		?><div id="sq_backend_sections"><?php
		if (!$this->_redirect) $this->paintSections();
		?></div><?php

		$this->_paintBodyFooter();

	}//end paint()


	/**
	* Opens a section of the page for raw output
	* we are overwritting the closeRaw so it handles non-buffering and limbo
	*
	* @return void
	* @access public
	*/
	function openRaw()
	{
		if ((!$this->_buffering) && SQ_IN_LIMBO) {
			parent::openSection('&nbsp;');
		} else {
			parent::openRaw();
		}

	}//end openRaw()


	/**
	* Closes a section of the page for raw output
	* we are overwritting the closeRaw so it handles non-buffering and limbo
	*
	* @return void
	* @access public
	*/
	function closeRaw()
	{
		if ((!$this->_buffering) && SQ_IN_LIMBO) {
			parent::closeSection();
		} else {
			parent::closeRaw();
		}

	}//end closeRaw()


	/**
	* Paint the sections within out contents
	*
	* @return void
	* @access public
	*/
	function paintSections()
	{
		for ($i = 0; $i < count($this->_contents); $i++) {
			$this->_paintSection($this->_contents[$i]);
		}

	}//end paintSections()


	/**
	* Paint the header of the page as it should appear in the <head> of the document
	*
	* @return void
	* @access private
	*/
	function _paintHeader()
	{
		// Charset header
		if (!headers_sent()) {
			if ($this->_charset) {
				header("Content-type: text/html; charset=$this->_charset");
			} else {
				$default_charset = SQ_CONF_DEFAULT_CHARACTER_SET;
				header("Content-type: text/html; charset=$default_charset");
			}
		}

		// CSS includes
		// Let's get Layout Manager to see if we should output the edit.css file
		$lm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');
		if ($lm->attr('edit_css')) {
			$this->addCssInclude($this->filesPath('css/limbo.css'));
		}

		$this->loadViperEditor();
			
		foreach ($this->_css_includes as $file) {
			$this->_paintCssInclude($file);
		}

		// first add the js translation files
		foreach ($GLOBALS['SQ_SYSTEM']->lm->getJavascriptIncludes() as $js_include) {
			$this->addJsInclude($js_include);
		}

		// JS includes, including some defaults
		$this->addJsInclude(sq_web_path('lib').'/html_form/html_form.js');
		$this->addJsInclude(sq_web_path('lib').'/js/general.js');
		$this->addJsInclude(sq_web_path('lib').'/js/debug.js');
		$this->addJsInclude(sq_web_path('lib').'/js/edit.js');
		$this->addJsInclude(sq_web_path('lib').'/js/translation.js');
		$this->addJsInclude(sq_web_path('lib').'/js/tooltip.js');

		list($lang, $country, $variant) = $GLOBALS['SQ_SYSTEM']->lm->getLocaleParts($GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale());

		if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'.js')) {
				$this->addJsInclude(sq_web_path('data').'/system/core/js_strings.'.$lang.'.js');
		}

		if (!empty($country)) {
			if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'_'.$country.'.js')) {
				$this->addJsInclude(sq_web_path('data').'/system/core/js_strings.'.$lang.'_'.$country.'.js');
			}

			if (!empty($variant)) {
				if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'_'.$country.'@'.$variant.'.js')) {
					$this->addJsInclude(sq_web_path('data').'/system/core/js_strings.'.$lang.'_'.$country.'@'.$variant.'.js');
				}
			}
		}

		foreach ($this->_js_includes as $file) {
			$this->_paintJsInclude($file);
		}
		
		// Event handlers
		$this->_paintEventHandlers();

		// Preloaded images
		foreach ($this->_preload_imgs as $file) {
			echo "\t\tpreload_image('".$file."');\n";
		}

	}//end _paintHeader()


	/**
	* Paint the header as it would appear if the contents of this ouputter were being printed in the <body> of a document
	*
	* @return void
	* @access public
	*/
	function _paintInlineHeader()
	{
		// CSS includes (if not empty), inline style
		if (!empty($this->_css_includes)) {
		?>
		<style type="text/css">
			<?php
			foreach ($this->_css_includes as $file) {
				echo "\t\t@import url($file)\n";
			}
			?>
		</style>
		<?php
		} // End if

		// JS includes, including some defaults
		$this->addJsInclude(sq_web_path('lib').'/html_form/html_form.js');
		$this->addJsInclude(sq_web_path('lib').'/js/general.js');
		$this->addJsInclude(sq_web_path('lib').'/js/debug.js');
		$this->addJsInclude(sq_web_path('lib').'/js/edit.js');
		$this->addJsInclude(sq_web_path('lib').'/js/translation.js');

		list($lang, $country, $variant) = $GLOBALS['SQ_SYSTEM']->lm->getLocaleParts($GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale());

		if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'.js')) {
				$this->addJsInclude(sq_web_path('data').'/system/core/js_strings.'.$lang.'.js');
		}

		if (!empty($country)) {
			if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'_'.$country.'.js')) {
				$this->addJsInclude(sq_web_path('data').'/system/core/js_strings.'.$lang.'_'.$country.'.js');
			}

			if (!empty($variant)) {
				if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$lang.'_'.$country.'@'.$variant.'.js')) {
					$this->addJsInclude(sq_web_path('data').'/system/core/js_strings.'.$lang.'_'.$country.'@'.$variant.'.js');
				}
			}
		}

		foreach ($this->_js_includes as $file) {
			$this->_paintJsInclude($file);
		}

		// Event handlers
		$this->_paintEventHandlers();

		// Hidden Fields
		$this->_paintHiddenFields();

		// Preload images
		foreach ($this->_preload_imgs as $file) {
			echo "\t\tpreload_image('".$file."');\n";
		}

	}//end _paintInlineHeader()


	/**
	* Print the JS that attaches the specified onload and onsubmit handlers
	*
	* @return void
	* @access public
	*/
	function _paintEventHandlers()
	{
		?>
		<script type="text/javascript"><!--
		//<![CDATA[

			// slip our own onLoad function into the onLoad event of the window
			var SQ_DOCUMENT_LOADED = false;

			function page_on_load()
			{
				<?php
				foreach ($this->_on_load_calls as $call) {
					echo "\t\t$call;\n";
				}//end foreach
				?>

				// let everyone know the document is loaded
				SQ_DOCUMENT_LOADED = true;

			}// end page_on_load()

			function print_tooltip()
			{
				if (window.self.name == 'sq_main') {
					tooltip.print();
				}
			}

			var SQ_FORM_SUBMITTED = false;

			function form_on_submit()
			{
				if (SQ_FORM_SUBMITTED) {
					alert(js_translate('form_already_submitted'));
					return;
				}

				<?php
					foreach ($this->_on_submit_calls as $call) {
						echo "\t\t$call;\n";
					}
				?>

				// basically if they get this far then we can submit
				SQ_FORM_SUBMITTED = true;
				return true;

			}//end form_on_submit()

		//]]> -->
		</script>
		<?php

	}//end _paintEventHandlers()


	/**
	* Print a field without buffering (nb == non-buffering)
	*
	* @param string $name		label of the field
	* @param string $format		format 'new_line' | 'blank' | 'commit' | ''
	* @param string $note		a note to place with the field
	* @param boolean $hidden	boolean
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
						<td class="sq-limbo-field" colspan="2">
							<a name="field_<?php echo $this->_field_count; ?>"></a>
							<?php echo $name;?>
						</td>
					</tr>
					<tr <?php if ($hidden) echo 'style="display: none"'; ?>>
						<td class="sq-backend-data sq-backend-data-newline" colspan="2">

				<?php
			break;

			case 'commit' :
				?>
				<tr>
					<td class="sq-backend-data sq-backend-commit" colspan="2">
				<?php
			break;

			case 'blank' :
				// a blank field to be used just for formatting
			break;

			default :
			?>
				<tr <?php if ($hidden) echo 'style="display: none"'; ?>>
					<td class="sq-limbo-field">
						<a name="field_<?php echo $this->_field_count; ?>"></a>
						<?php echo $name;?>
					</td>
					<td class="sq-backend-data">
			<?php
		}//end switch

	}//end _nbOpenField()


	/**
	* Opens a section without buffering
	*
	* @param array		$section	some initialising data
	*
	* @return void
	* @access private
	*/
	function _nbOpenSection($section)
	{
		if ($this->_nb_section_count > 0) echo '<tr><td>';
		$this->_nb_section_count++;

		?>
			<a name="section_<?php echo $this->_nb_section_count; ?>"></a>
			<table class="sq-backend-section-table">
			<?php
			if ($section['heading']) {
				if ($this->_is_open_section) {
					// printing a nested section - needs to look a little different
					?>
					<tr>
						<td class="sq-limbo-section-subheading" colspan="2"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				} else {
					$this->_is_open_section = TRUE;
					// printing a top level section
					?>
					<tr>
						<td class="sq-limbo-section-heading" colspan="2"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				}
			}//end if

			// print the section seperator
			?>
			<tr>
				<td>
					<table class="sq-backend-section-table-inner">

			<?php

	}//end _nbOpenSection()


	/**
	* Paint the passed section onto the page
	*
	* @param array $section	section
	* @param int $depth		depth
	*
	* @return void
	* @access private
	*/
	function _paintSection($section, $depth=0)
	{
		if ($section['type'] == 'raw') {
			echo $section['contents'];
		} else {
			?>
			<a name="section_<?php echo $section['section_count']; ?>"></a>
			<table class="sq-backend-section-table">
			<?php
			if ($section['heading']) {
				if ($depth > 0) {
					// printing a nested section - needs to look a little different
					?>
					<tr>
						<td class="sq-limbo-section-subheading"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				} else {
					// printing a top level section
					?>
					<tr>
						<td class="sq-limbo-section-heading" style="border-bottom: 1px solid #000000"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				}
			}//end if

			// print the section seperator
			?>
			<tr>
				<td>
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
						$field =& $section['areas'][$j];
						switch ($field['format']) {
							case 'new_line' :
								?>
									<tr <?php if ($field['hidden']) echo 'style="display: none"'; ?>>
										<td class="sq-limbo-field" colspan="2">
											<a name="field_<?php echo $field['field_count']; ?>"></a>
											<?php echo $field['name'];?>
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
								<tr <?php if ($field['hidden']) echo 'style="display: none"'; ?>>
									<td class="sq-backend-data sq-limbo-data sq-backend-commit" colspan="2">
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
								<tr <?php if ($field['hidden']) echo 'style="display: none"'; ?>>
									<td class="sq-limbo-field">
										<a name="field_<?php echo $field['field_count']; ?>"></a>
										<?php echo $field['name'];?>
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

			?>
					</table>
				</td>
			</tr>
			</table>
			<?php

		}//end else if section type == raw

	}//end _paintSection()


	/**
	* Paint the footer of the page
	*
	* @return void
	* @access private
	*/
	function _paintBodyHeader()
	{
		// if a redirect has been set - add an onLoad call so we redirect as normal
		?>
		<script type="text/javascript"><!--
		//<![CDATA[
			<?php
			if ($this->_redirect) {
				echo $this->_redirect_target.'.location = "'.addslashes($this->_redirect).'";';
			} else {
				echo '
					  current_onload = (window.onload) ? window.onload : new Function;
					  window.onload = new Function ("page_on_load(); current_onload(); print_tooltip();")';
			}
		?>

		//]]> -->
		</script>


		<form action="<?php echo $this->_action.$this->_anchor;?>" id="main_form" name="main_form" method="post" enctype="multipart/form-data" onsubmit="return form_on_submit();">

		<?php
		hidden_field('process_form', '1');
		// insert nonce token
		hidden_field('token', get_unique_token());
		$this->_paintHiddenFields();

	}//end _paintBodyHeader()


	/**
	* Paint the buffered hidden fields
	*
	* @return void
	* @access public
	*/
	function _paintHiddenFields()
	{
		foreach ($this->_hidden_fields as $name => $value) {
			hidden_field($name, $value);
		}

	}//end _paintHiddenFields()


	/**
	* Paint the footer of the page
	*
	* @return void
	* @access private
	*/
	function _paintBodyFooter()
	{
		?>
		</form>
		<?php

	}//end _paintBodyFooter()


}//end class

?>
