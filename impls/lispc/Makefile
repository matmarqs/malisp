CC = gcc
CFLAGS = -std=c99 -Og -g -Wall $(shell pkg-config --cflags libedit libpcre2-8)
LDFLAGS =  $(shell pkg-config --libs libedit libpcre2-8)

all: step6_file

step0_repl: step0_repl.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

step1_read_print: step1_read_print.c reader.c mal_obj.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

step2_eval: step2_eval.c reader.c mal_obj.c fun_hash.c str.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

step3_env: step3_env.c reader.c mal_obj.c builtin.c str.c env.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

step4_if_fn_do: step4_if_fn_do.c reader.c mal_obj.c core.c str.c env.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

step5_tco: step5_tco.c reader.c mal_obj.c core.c str.c env.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

step6_file: step6_file.c reader.c mal_obj.c core.c str.c env.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f step0_repl step1_read_print step2_eval step3_env step4_if_fn_do step5_tco step6_file
