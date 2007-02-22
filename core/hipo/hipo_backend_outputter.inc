<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: hipo_backend_outputter.inc,v 1.21 2007/02/22 00:16:00 tbarrett Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/include/backend_outputter.inc';


/**
* Backend_Outputter
*
* Purpose
*
*    similar to the standard backend outputter, but colour scheme modified to
*    suite hipo jobs
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix
*/
class Hipo_Backend_Outputter extends Backend_Outputter
{

	/**
	* An array of data decribing the main HIPO form and its action
	* @var array
	*/
	var $_form_data = Array();


	/**
	* Constructor
	*
	* @param string	$type_code
	*
	* @access  public
	*/
	function Hipo_Backend_Outputter()
	{
		$this->Backend_Outputter();

	}//end constructor


	/**
	* Set the form name and url to submit to
	*
	* @param string	$form_name	the name of the form
	* @param string	$url		the url to submit to
	*
	* @return void
	* @access public
	*/
	function setFormData($form_name, $url)
	{
		$this->_form_data = Array('form_name' => $form_name, 'url' => $url);

	}//end setFormData()


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
				header("Content-type: text/html; charset=$this->_charset");
			} else {
				header('Content-type: text/html; charset='.SQ_CONF_DEFAULT_CHARACTER_SET);
			}
		}
		?>
		<html>
			<head>
				<title><?php echo translate('hipo_processing'); ?></title>
				<?php

				$this->addCssInclude($this->filesPath('css/edit.css'));
				$this->addCssInclude($this->filesPath('css/hipo.css'));
				foreach ($this->_css_includes as $file) {
					$this->_paintCssInclude($file);
				}

				// add JS translation files
				foreach ($GLOBALS['SQ_SYSTEM']->lm->getJavascriptIncludes() as $js_include) {
					$this->addJsInclude($js_include);
				}

				// add general Js includes
				$this->addJsInclude(sq_web_path('lib').'/html_form/html_form.js');
				$this->addJsInclude(sq_web_path('lib').'/js/general.js');
				$this->addJsInclude(sq_web_path('lib').'/js/debug.js');
				$this->addJsInclude(sq_web_path('lib').'/js/edit.js');

				// print them all out
				foreach ($this->_js_includes as $file) {
					$this->_paintJsInclude($file);
				}
				?>
				<script type="text/javascript">
				<!--
					var ON_LOAD_TIME_OUT = null;
					function page_on_load() {
						<?php
						foreach ($this->_on_load_calls as $call) {
							echo "\n".$call;
						}
						?>
					}// end page_on_load()

				// -->
				</script>
			</head>
			<body bgcolor="#FFFFFF" onLoad="javascript: page_on_load();">
				<table border="0" cellspacing="0" cellpadding="0" width="100%">
					<tr>
						<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="20" /></td>
					</tr>
					<tr>
						<td align="center">
						<form name="<?php echo $this->_form_data['form_name']; ?>" method="post" action="<?php echo $this->_form_data['url']; ?>"  enctype="multipart/form-data">
					<?php
					foreach ($this->_hidden_fields as $name => $value) {
						hidden_field($name, $value);
					}

	}//end _paintHeader()


	/**
	* Paint the footer of the page
	*
	* @return void
	* @access private
	*/
	function _paintFooter()
	{
		?>
					</form>
					</td>
				</tr>
			</table>
			</body>
		</html>
		<?php

	}//end _paintFooter()


	/**
	* Paint the passed section onto the page
	*
	* @param string	$section	the type of the section
	* @param int	$depth		the number of nested steps
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
			<table width="100%" cellpadding="0" cellspacing="0" border="0">
			<?php
			if ($section['heading']) {
				if ($depth > 0) {
					// printing a nested section - needs to look a little different
					?>
					<tr>
						<td class="sq-backend-section-subheading" colspan="3"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				} else {
					// printing a top level section
					?>
					<tr class="sq-backend-section-heading-container">
						<td colspan="3">
							<table border="0" cellspacing="0" cellpadding="0">
								<tr>
									<td class="sq-backend-section-heading"><?php echo $section['heading'];?></td>
									<td><img src="<?php echo $this->filesPath('images/section_icon.gif');?>" width="27" height="21" /></td>
								</tr>
							</table>
						</td>
					</tr>
					<?php
				}
			}//end if

			// print the section seperator
			?>
			<tr>
				<td colspan="2"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td>
			</tr>
			<tr>
				<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
				<td width="100%">
					<table cellspacing="0" cellpadding="0" border="0" width="100%">
			<?php

			for ($j = 0; $j < count($section['areas']); $j++) {
				switch ($section['areas'][$j]['area_type']) {
					case 'section' :
						?>
							<tr>
								<td width="100%" valign="top" colspan="2">
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
								$style = $id = '';
								if ($field['hidden']) {
									$style = 'style="display: none"';
								}
								if ($field['id_name']) {
									$id = 'id="'.$field['id_name'].'"';
								}
								?>
									<tr <?php echo $style; ?> <?php echo $id; ?>>
										<td class="sq-backend-field sq-backend-field-newline" colspan="2">
											<a name="field_<?php echo $field['field_count']; ?>"></a>
											<?php echo $field['name'];?>
										</td>
									</tr>
									<tr>
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
								<tr>
									<td class="sq-backend-data" style="text-align: right;" colspan="2">
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
								$style = $id = '';
								if ($field['hidden']) {
									$style = 'style="display: none"';
								}
								if ($field['id_name']) {
									$id = 'id="'.$field['id_name'].'"';
								}
							?>
								<tr <?php echo $style; ?> <?php echo $id; ?>>
									<td class="sq-backend-field">
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
						trigger_localised_error('HIPO0059', E_USER_ERROR, $section['areas'][$j]['area_type']);

				}//end switch area type

			}//end for

			?>
					</table>
				</td>
				<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
			</tr>
			</table>
			<table width="100%" cellpadding="0" cellspacing="0" border="0"><tr><td width="100%"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td></tr></table>
			<?php

		}//end else section type == raw

	}//end _paintSection()


}//end class

?>