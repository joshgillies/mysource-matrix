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
* $Id: limbo_outputter.inc,v 1.5.2.4 2004/04/21 10:55:16 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';

/**
* Limbo_Outputter
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
	function addFormActionGetVar($name, $value, $persistant=false)
	{
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
		while(!empty($this->_section_stack)) $this->closeSection();
		$this->_paintBodyHeader();
		?><div id="sq_backend_sections"><?php
		if (!$this->_redirect) $this->paintSections();
		?></div><?php

		$this->_paintBodyFooter();

	}//end paint()


	/**
	* Paint the sections within out contents
	*
	* @return void
	* @access public
	*/
	function paintSections()
	{
		for($i = 0; $i < count($this->_contents); $i++) {
			$this->_paintSection($this->_contents[$i]);
		}

	}//end paintSections()


	/**
	* Paint the header of the page
	*
	* @access private
	*/
	function _paintHeader()
	{
		if (!headers_sent()) {
			if($this->_charset) {
				header("Content-type: text/html; charset=$this->_charset");
			} else {
				$default_charset = 'iso-8859-1';
				header("Content-type: text/html; charset=$default_charset");
			}
		}

		$this->_paintCssInclude($this->filesPath('css/edit.css'));
		foreach($this->_css_includes as $file) $this->_paintCssInclude($file);

		$this->_paintJsInclude(sq_web_path('lib').'/html_form/html_form.js');
		$this->_paintJsInclude(sq_web_path('lib').'/js/debug.js');
		$this->_paintJsInclude(sq_web_path('lib').'/js/edit.js');
		foreach($this->_js_includes as $file) $this->_paintJsInclude($file);
		
		?>
		<script language="Javascript" type="text/javascript">
		<!--
			// slip our own onLoad function into the onLoad event of the window
			var SQ_DOCUMENT_LOADED = false;
			if (window.onload) {
				var sqOtherOnLoad = window.onload;
				window.onload = function() { sqOtherOnLoad(); page_on_load(); }
			} else {
				window.onload = function() { page_on_load(); }
			}

			function page_on_load() {

				<?php
				foreach($this->_on_load_calls as $call) {
					echo "\n".$call;
				}//end foreach

				?>

				// let everyone know the document is loaded
				SQ_DOCUMENT_LOADED = true;

			}// end page_on_load()


			var SQ_FORM_SUBMITTED = false;
			function form_on_submit() {
				if (SQ_FORM_SUBMITTED) {
					alert('The form has already been submitted, please wait ...');
					return;
				}

				<?php
					foreach($this->_on_submit_calls as $call) {
					?>
						<?php echo $call;?>;
					<?php
					}//end foreach
				?>

				// basically if they get this far then we can submit
				SQ_FORM_SUBMITTED = true;
				return true;

			}// end form_on_submit()

			<?php
			foreach($this->_preload_imgs as $file) {
			?>
				preload_image('<?php echo $file;?>');
			<?php
			}//end foreach
			?>
		// -->
		</script>
		<?php

	}//end _paintHeader()


	/**
	* Print a field without buffering (nb == non-buffering)
	*
	* @param string $name		label of the field
	* @param string $format		format 'new_line' | 'blank' | 'commit' | ''
	* @param string $note		a note to place with the field
	*
	* @return boolean
	* @access private
	*/
	function _nbOpenField($name, $format='', $note='')
	{
		if ($this->_is_open_field) $this->closeField();
		$this->_is_open_field = true;

		// store the format and note of the current field for future reference
		$this->_current_field_type = $format;
		$this->_current_field_note = $note;

		switch ($format) {
			case 'new_line' :
				?>
					<tr>
						<td nowrap valign="top" class="sq-limbo-field" colspan="3">
							<a name="field_<?php echo $this->_field_count; ?>"></a>
							<?php echo $name;?>
						</td>
					</tr>
					<tr>
						<td valign="top" class="sq-backend-data" colspan="3">
							<table cellpadding="0" cellspacing="0" border="0">
								<tr>
									<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="5" height="1"/></td>
									<td width="100%" valign="top" class="sq-backend-data">
				<?php
			break;
	
			case 'commit' :
				?>
				<tr>
					<td width="100%" valign="top" class="sq-backend-data" style="text-align: right; border" colspan="3">
				<?php
			break;
	
			case 'blank' :
				// a blank field to be used just for formatting
			break;
	
			default :
			?>
				<tr>
					<td nowrap valign="top" class="sq-limbo-field">
						<a name="field_<?php echo $this->_field_count; ?>"></a>
						<?php echo $name;?>
					</td>
					<td width="10"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
					<td width="100%" valign="top" class="sq-backend-data">
			<?php
		}//end switch

	}//end _nbOpenField()

	
	/**
	* Opens a section without buffering
	*
	* @param Array		$section	some initialising data
	*
	* @return void
	* @access private
	*/
	function _nbOpenSection($section)
	{
		$this->_nb_section_count++;
		?>
			<a name="section_<?php echo $this->_nb_section_count; ?>"></a>
			<table cellpadding="0" cellspacing="1" border="0">
			<?php
			if ($section['heading']) {
				if ($this->_is_open_section) {
					// printing a nested section - needs to look a little different
					?>
					<tr>
						<td class="sq-limbo-section-subheading" colspan="3"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				} else {
					$this->_is_open_section = true;
					// printing a top level section
					?>
					<tr>
						<td class="sq-limbo-section-heading" colspan="3" style="border-bottom: 1px solid #000000"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				}
			}//end if

			// print the section seperator
			?>
			<tr>
				<td colspan="3"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td>
			</tr>
			<tr>
				<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
				<td width="100%">
					<table cellspacing="0" cellpadding="0" border="0" width="100%">
						<tr>
							<td width="100%" valign="top" colspan="3">
			<?php

	}//end _nbOpenSection()


	/**
	* Paint the passed section onto the page
	*
	* @access private
	*/
	function _paintSection($section, $depth=0)
	{
		if ($section['type'] == 'raw') {
			echo $section['contents'];
		} else {

		?>
			<a name="section_<?php echo $section['section_count']; ?>"></a>
			<table cellpadding="0" cellspacing="1" border="0">
			<?php
			if ($section['heading']) {
				if ($depth > 0) {
					// printing a nested section - needs to look a little different
					?>
					<tr>
						<td class="sq-limbo-section-subheading" colspan="3"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				} else {
					// printing a top level section
					?>
					<tr>
						<td class="sq-limbo-section-heading" colspan="3" style="border-bottom: 1px solid #000000"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				}
			}//end if

			// print the section seperator
			?>
			<tr>
				<td colspan="3"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td>
			</tr>
			<tr>
				<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
				<td width="100%">
					<table cellspacing="0" cellpadding="0" border="0" width="100%">
			<?php

			for($j = 0; $j < count($section['areas']); $j++) {
				switch($section['areas'][$j]['area_type']) {
					case 'section' :
						?>
							<tr>
								<td width="100%" valign="top" colspan="3">
								<?php
									$this->_paintSection($section['areas'][$j], $depth + 1);
								?>
								</td>
							</tr>
						<?php

						break;

					case 'field' :
						$field = &$section['areas'][$j];
						switch ($field['format']) {
							case 'new_line' :
								?>
									<tr>
										<td nowrap valign="top" class="sq-limbo-field" colspan="3">
											<a name="field_<?php echo $field['field_count']; ?>"></a>
											<?php echo $field['name'];?>
										</td>
									</tr>
									<tr>
										<td valign="top" class="sq-backend-data" colspan="3">
											<table cellpadding="0" cellspacing="0" border="0">
												<tr>
													<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="5" height="1"/></td>
													<td width="100%" valign="top" class="sq-backend-data">
													<?php
														echo $field['contents'];
														if ($field['note']) $this->note($field['note']);
													?>
													</td>
												</tr>
											</table>
										</td>
									</tr>
								<?php
							break;

							case 'commit' :
								?>
								<tr>
									<td width="100%" valign="top" class="sq-backend-data" style="text-align: right; border" colspan="3">
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
								<tr>
									<td nowrap valign="top" class="sq-limbo-field">
										<a name="field_<?php echo $field['field_count']; ?>"></a>
										<?php echo $field['name'];?>
									</td>
									<td width="10"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
									<td width="100%" valign="top" class="sq-backend-data">
									<?php
										echo $field['contents'];
										if ($field['note']) $this->note($field['note']);
									?>
									</td>
								</tr>
								<tr>
									<td colspan="3"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td>
								</tr>
							<?php
						}//end switch

						break;

					default :
						trigger_error('Unknown Area Type : "'.$section['areas'][$j]['area_type'].'"', E_USER_ERROR);

				}// end switch area type

			}//end for

			?>
					</table>
				</td>
				<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
			</tr>
			</table>
			<table cellpadding="0" cellspacing="0" border="0"><tr><td width="100%"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td></tr></table>
			<?php

		}//end if section type == raw

	}//end _paintSection()


	/**
	* Paint the footer of the page
	*
	* @access private
	*/
	function _paintBodyHeader()
	{
		// if a redirect has been set - add an onLoad call so we redirect as normal
		if ($this->_redirect) {
			?>
			<script language="Javascript" type="text/javascript">
				<?php echo $this->_redirect_target; ?>.location = "<?php echo addslashes($this->_redirect); ?>";
			</script>
			<?php
		}
		?>

		<!--
			some special styles we need in limbo that dont get
			overwritten by other stylsheets - thus the reason why we define
			then within the source instead of in the backend css
		-->
		<style type="text/css">
			#sq_backend_sections td {
				border:			0px;
				margin:			0px;
				padding:		0px;
				text-align:		left;
				vertical-align:	top;
			}
		</style>

		<form action="<?php echo $this->_action.$this->_anchor;?>" name="main_form" method="post" enctype="multipart/form-data" onSubmit="javascript: return form_on_submit();">
		<?php

		hidden_field('process_form', '1');
		foreach($this->_hidden_fields as $name => $value) hidden_field($name, $value);

	}//end _paintBodyHeader()

	
	
	/**
	* Paint the footer of the page
	*
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