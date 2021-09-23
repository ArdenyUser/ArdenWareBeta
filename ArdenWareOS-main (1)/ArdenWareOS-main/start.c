#include start.h

int cake_base_three() 
{
  char ch = 0;
  char cha = 0;
  char chb = 0;
  char keycode = 0;
  do{
    keycode = get_input_keycode();
    if(keycode == KEY_ENTER){
      print_new_line();
    }else{
      ch = get_ascii_char(keycode);
      print_char(ch);
      do{
        keycode = get_input_keycode();
        if(keycode == KEY_ENTER){
           print_new_line();
        }else{
          cha = get_ascii_char(keycode);
          print_char(cha);
          do{
            keycode = get_input_keycode();
            if(keycode == KEY_ENTER){
               print_new_line();
            }else{
              cha = get_ascii_char(keycode);
              print_char(cha);
            }
            sleep(0x02FFFFFF);
            }while(ch > 0);
        }
        sleep(0x02FFFFFF);
         }while(ch > 0);
    }
    sleep(0x02FFFFFF);
    }while(ch > 0);
          
}             // FIX BUGS tho i think they fixed   
                
void setup() 
{
  print_new_line();
  init_vga(GREEN, BLACK);
  print_new_line();
  print_string("DogeBakery Compiler Version One");
  print_new_line();

}
