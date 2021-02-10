ELFBIT ?= 32
TARGET := scexec

SRCDIR := src
OBJDIR := obj
INCDIR := $(SRCDIR)/include

LDFLAG := -z execstack
CCWARN := -Wall
CCFLAG := -m$(ELFBIT) -fno-stack-protector

OBJALL := $(OBJDIR)/shannon$(ELFBIT).o $(OBJDIR)/main$(ELFBIT).o

$(TARGET)$(ELFBIT): $(OBJALL)
	@echo "[$(CC)] $(CCFLAG) $(LDFLAG) $@"
	@$(CC) $(CCWARN) $(CCFLAG) $(LDFLAG) -o $@ $(OBJALL) -lm

$(OBJDIR):
	@mkdir $(OBJDIR)

$(OBJDIR)/main$(ELFBIT).o: $(SRCDIR)/main.c
	@echo "[$(CC)] -O2 $@"
	@$(CC) $(CCWARN) -I$(INCDIR) $(CCFLAG) -D'TARGET="$(TARGET)$(ELFBIT)"' -O2 -c -o $@ $<

$(OBJDIR)/shannon$(ELFBIT).o: $(SRCDIR)/shannon.c $(INCDIR)/shannon.h
	@echo "[$(CC)] -Ofast $@"
	@$(CC) $(CCWARN) -I$(INCDIR) $(CCFLAG) -Ofast -c -o $@ $<

$(OBJALL): | $(OBJDIR)

setuid:
	@echo "[chown] root:root $(TARGET)$(ELFBIT)"
	@chown root:root $(TARGET)$(ELFBIT)
	@echo "[chmod] u+s $(TARGET)$(ELFBIT)"
	@chmod u+s $(TARGET)$(ELFBIT)

clean:
	@echo "[rm] $(TARGET)* $(OBJDIR)"
	@rm -rf $(TARGET)* $(OBJDIR) || true

.PHONY: setuid clean
