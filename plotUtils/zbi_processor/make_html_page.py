import os
import argparse

# Function to extract a key for sorting
def sorting_key(filename):
    parts = filename.split('_')
    if parts[1] == "initial":
        return (0, filename)
    elif parts[1].isdigit():
        return (int(parts[1]), filename)
    else:
        return (1, filename)  # For non-integer keys, keep them in their original order

def make_parent_html(base_dir, name):
    os.makedirs(base_dir, exist_ok=True)
    with open(os.path.join(base_dir, '%s.html'%(name)), 'w') as f:
        f.write(f'''
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>{name} ZBi Processor Results</title>
    </head>
    <body>
        <h1> {name} ZBi Plots</h1>
        <p>Click on the links below to explore variables and iterations:</p>
        
        <ul>
            <li><a href="variables.html">variables</a></li>
            <li><a href="iterations.html">iterations</a></li>
        </ul>
    </body>
    </html>
        ''')

def make_variables_html(parent_dir, top_name, name):
    base_dir = os.path.join(parent_dir, name)
    basename = os.path.basename(base_dir)
    os.makedirs(base_dir, exist_ok=True)
    with open(os.path.join(parent_dir, '%s.html'%(name)), 'w') as f:
        f.write(f'''
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>{top_name} Page</title>
    </head>
    <body>
        <h1>{top_name} - {name} </h1>
        <p>Click 1D or 2D plots:</p>
        
        <ul>
            <li><a href="{basename}/plots_1D.html">1D Plots</a></li>
            <li><a href="{basename}/plots_2D.html">2D Plots</a></li>
        </ul>
    </body>
    </html>
        ''')

    return base_dir

def make_var_1d_plots(parent_dir, top_name, name, input_images_dir, keyword):

    #Get list of available variables from image names
    #input_images_dir = '/sdf/group/hps/users/alspellm/projects/THESIS/ana/zbi_html/zalpha_plots_html_test/1d_plots'
    variables = [png for png in sorted(os.listdir(input_images_dir)) if keyword in png]
    variables = [os.path.basename(png).replace(keyword,'').replace('.png','') for png in variables]
    print("List of available variables: ", variables)

    #Make 1D Plot subdirectory
    base_dir = os.path.join(parent_dir, name)
    basename = os.path.basename(base_dir)
    os.makedirs(base_dir, exist_ok=True)

    #Write html file with a link to the html page of each variable
    with open(os.path.join(parent_dir, '%s.html'%(name)), 'w') as f:
        #Automatically generate links for every variable in list of variables
        links = "\n".join([f'<li><a href="{basename}/{var}.html">{var}</a></li>' for var in variables])
        f.write(f'''
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>{top_name} - Variables - 1D Plots As Function of Iteration</title>
    </head>
    <body>
        <h1>Signal and Background Variables for {top_name}</h1>
        <p> Click a variable to see how Signal and Background change with iterative cuts </p>
        <ul>
            {links}
        </ul>
    </body>
    </html>
        ''')

    #Write an html file for each variable that contains links to variable plots
    for var in variables:
        with open(os.path.join(base_dir, f'{var}.html'), 'w') as f:

            #Generate links to variable plots
            matching_pngs = [png for png in sorted(os.listdir(input_images_dir)) if var in png]
            sorted_pngs = sorted(matching_pngs, key=sorting_key)
            # Generate div containers with clickable img tags for the sorted PNG files
            img_divs = "\n".join([
                f'<div><a href="{input_images_dir}/{png}" target="_blank"><img src="{input_images_dir}/{png}" alt="{png}" width="1000"></a></div>'
                for png in sorted_pngs
            ])

            f.write(f'''
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>{top_name} {var} 1D Plots By Iteration</title>
        <style>
            div {{
                margin-bottom: 20px; /* Add spacing between images */
            }}
        </style>
    </head>
    <body>
        <h1></h1>
        <ul>
            {img_divs}
        </ul>
    </body>
    </html>
            ''')


def make_var_2d_plots(parent_dir, top_name, name, input_images_dir, keyword):

    #Get list of available variables from image names
    input_images_dir = "/sdf/group/hps/users/alspellm/projects/THESIS/ana/zbi_html/zalpha_plots_html_test/2d_plots"
    variables = [png for png in sorted(os.listdir(input_images_dir)) if keyword in png]
    variables = [os.path.basename(png).replace(keyword,'').replace('.png','') for png in variables]
    variables.append("recon_z")
    print("List of available variables: ", variables)

    #Make Plot subdirectory
    base_dir = os.path.join(parent_dir, name)
    basename = os.path.basename(base_dir)
    os.makedirs(base_dir, exist_ok=True)

    #Write html file with a link to the html page of each variable
    with open(os.path.join(parent_dir, '%s.html'%(name)), 'w') as f:
        #Automatically generate links for every variable in list of variables
        links = "\n".join([f'<li><a href="{basename}/{var}.html">{var}</a></li>' for var in variables])
        f.write(f'''
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>{top_name} - Variables - 2D Plots As Function of Iteration</title>
    </head>
    <body>
        <h1>Signal and Background Variables for {top_name}</h1>
        <p> Click a variable to see how Signal and Background change with iterative cuts </p>
        <ul>
            {links}
        </ul>
    </body>
    </html>
        ''')

    #Write an html file for each variable that contains links to variable plots
    for var in variables:
        with open(os.path.join(base_dir, f'{var}.html'), 'w') as f:

            #Generate links to variable plots
            matching_pngs = [png for png in sorted(os.listdir(input_images_dir)) if var in png]
            sorted_pngs = sorted(matching_pngs, key=sorting_key)
            # Generate div containers with clickable img tags for the sorted PNG files
            img_divs = "\n".join([
                f'<div><a href="{input_images_dir}/{png}" target="_blank"><img src="{input_images_dir}/{png}" alt="{png}" width="1000"></a></div>'
                for png in sorted_pngs
            ])

            f.write(f'''
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>{top_name} {var} 2D Plots By Iteration</title>
        <style>
            div {{
                margin-bottom: 20px; /* Add spacing between images */
            }}
        </style>
    </head>
    <body>
        <h1></h1>
        <ul>
            {img_divs}
        </ul>
    </body>
    </html>
            ''')

###############################################################################################
parser = argparse.ArgumentParser(description="HTML Config")
parser.add_argument('--name', type=str, dest="name", default = "variable_html")
parser.add_argument('--dir', type=str, dest="base_dir", default = "html_test")
options = parser.parse_args()

# Define the base directory
top_dir = 'zalpha_0pt3_html'
top_name = 'zalpha_0pt3'

#Parent
make_parent_html(top_dir, top_name)

#Variables
var_dir = make_variables_html(top_dir, top_name, 'variables')

#Variable 1D Plots
input_images_dir = '/sdf/group/hps/users/alspellm/projects/THESIS/ana/zbi_html/zalpha_plots_html_test/1d_plots'
make_var_1d_plots(var_dir, top_name, 'plots_1D', input_images_dir, "iteration_initial_")

#Variable 2D Plots
input_images_dir = "/sdf/group/hps/users/alspellm/projects/THESIS/ana/zbi_html/zalpha_plots_html_test/2d_plots"
make_var_2d_plots(var_dir, top_name, 'plots_2D', input_images_dir, "initial_")


print("File structure and HTML pages have been created.")
